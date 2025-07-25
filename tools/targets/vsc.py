#
# File      : vsc.py
# This file is part of RT-Thread RTOS
# COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Change Logs:
# Date           Author       Notes
# 2018-05-30     Bernard      The first version
# 2023-03-03     Supperthomas Add the vscode workspace config file
# 2024-12-13     Supperthomas covert compile_commands.json to vscode workspace file
# 2025-07-05     Bernard      Add support for generating .vscode/c_cpp_properties.json 
#                             and .vscode/settings.json files
"""
Utils for VSCode
"""

import os
import json
import utils
import rtconfig
from SCons.Script import GetLaunchDir

from utils import _make_path_relative
def find_first_node_with_two_children(tree):
    for key, subtree in tree.items():
        if len(subtree) >= 2:
            return key, subtree
        result = find_first_node_with_two_children(subtree)
        if result:
            return result
    return None, None


def filt_tree(tree):
    key, subtree = find_first_node_with_two_children(tree)
    if key:
        return {key: subtree}
    return {}


def add_path_to_tree(tree, path):
    parts = path.split(os.sep)
    current_level = tree
    for part in parts:
        if part not in current_level:
            current_level[part] = {}
        current_level = current_level[part]


def build_tree(paths):
    tree = {}
    current_working_directory = os.getcwd()
    current_folder_name = os.path.basename(current_working_directory)
    # Filter out invalid and non-existent paths
    relative_dirs = []
    for path in paths:
        normalized_path = os.path.normpath(path)
        try:
            rel_path = os.path.relpath(normalized_path, start=current_working_directory)
            add_path_to_tree(tree, normalized_path)
        except ValueError:
            print(f"Remove unexcpect dir:{path}")

    return tree

def print_tree(tree, indent=''):
    for key, subtree in sorted(tree.items()):
        print(indent + key)
        print_tree(subtree, indent + '  ')

def extract_source_dirs(compile_commands):
    source_dirs = set()

    for entry in compile_commands:
        file_path = os.path.abspath(entry['file'])

        if file_path.endswith('.c'):
            dir_path = os.path.dirname(file_path)
            source_dirs.add(dir_path)
            # command or arguments
            command = entry.get('command') or entry.get('arguments')

            if isinstance(command, str):
                parts = command.split()
            else:
                parts = command
            # 读取-I或者/I
            for i, part in enumerate(parts):
                if part.startswith('-I'):
                    include_dir = part[2:] if len(part) > 2 else parts[i + 1]
                    source_dirs.add(os.path.abspath(include_dir))
                elif part.startswith('/I'):
                    include_dir = part[2:] if len(part) > 2 else parts[i + 1]
                    source_dirs.add(os.path.abspath(include_dir))

    return sorted(source_dirs)


def is_path_in_tree(path, tree):
    parts = path.split(os.sep)
    current_level = tree
    found_first_node = False
    root_key = list(tree.keys())[0]

    index_start = parts.index(root_key)
    length = len(parts)
    try:
        for i in range(index_start, length):
            current_level = current_level[parts[i]]
        return True
    except KeyError:
        return False


def generate_code_workspace_file(source_dirs,command_json_path,root_path):
    current_working_directory = os.getcwd()
    current_folder_name = os.path.basename(current_working_directory)

    relative_dirs = []
    for dir_path in source_dirs:
        try:
            rel_path = os.path.relpath(dir_path, root_path)
            relative_dirs.append(rel_path)
        except ValueError:
            continue

    root_rel_path = os.path.relpath(root_path, current_working_directory)
    command_json_path = os.path.relpath(current_working_directory, root_path) + os.sep
    workspace_data = {
        "folders": [
            {
                "path": f"{root_rel_path}"
            }
        ],
        "settings": {
            "clangd.arguments": [
                f"--compile-commands-dir={command_json_path}",
                "--header-insertion=never"
            ],
            "files.exclude": {dir.replace('\\','/'): True for dir in sorted(relative_dirs)}
        }
    }
    workspace_filename = f'{current_folder_name}.code-workspace'
    with open(workspace_filename, 'w') as f:
        json.dump(workspace_data, f, indent=4)

    print(f'Workspace file {workspace_filename} created.')

def command_json_to_workspace(root_path,command_json_path):
    
    with open('build/compile_commands.json', 'r') as f:
        compile_commands = json.load(f)

    source_dirs = extract_source_dirs(compile_commands)
    tree = build_tree(source_dirs)
    #print_tree(tree)
    filtered_tree = filt_tree(tree)
    print("Filtered Directory Tree:")
    #print_tree(filtered_tree)

    # 打印filtered_tree的root节点的相对路径
    root_key = list(filtered_tree.keys())[0]
    print(f"Root node relative path: {root_key}")

    # 初始化exclude_fold集合
    exclude_fold = set()

    # os.chdir(root_path)
    # 轮询root文件夹下面的每一个文件夹和子文件夹
    for root, dirs, files in os.walk(root_path):
        # 检查当前root是否在filtered_tree中
        if not is_path_in_tree(root, filtered_tree):
            exclude_fold.add(root)
            dirs[:] = []  # 不往下轮询子文件夹
            continue
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            if not is_path_in_tree(dir_path, filtered_tree):
                exclude_fold.add(dir_path)

    generate_code_workspace_file(exclude_fold,command_json_path,root_path)

def delete_repeatelist(data):
    temp_dict = set([str(item) for item in data])
    data = [eval(i) for i in temp_dict]
    return data

def GenerateCFiles(env):
    """
    Generate c_cpp_properties.json and build/compile_commands.json files
    """
    if not os.path.exists('.vscode'):
        os.mkdir('.vscode')

    with open('.vscode/c_cpp_properties.json', 'w') as vsc_file:
        info = utils.ProjectInfo(env)

        cc = os.path.join(rtconfig.EXEC_PATH, rtconfig.CC)
        cc = os.path.abspath(cc).replace('\\', '/')

        config_obj = {}
        config_obj['name'] = 'Linux'
        config_obj['defines'] = info['CPPDEFINES']

        intelliSenseMode = 'linux-gcc-arm'
        if cc.find('aarch64') != -1:
            intelliSenseMode = 'linux-gcc-arm64'
        elif cc.find('arm') != -1:
            intelliSenseMode = 'linux-gcc-arm'
        config_obj['intelliSenseMode'] = intelliSenseMode
        config_obj['compilerPath'] = cc
        config_obj['cStandard'] = "c99"
        config_obj['cppStandard'] = "c++11"
        config_obj['compileCommands'] ="build/compile_commands.json"

        # format "a/b," to a/b. remove first quotation mark("),and remove end (",)
        includePath = []
        for i in info['CPPPATH']:
            if i[0] == '\"' and i[len(i) - 2:len(i)] == '\",':
                includePath.append(_make_path_relative(os.getcwd(), i[1:len(i) - 2]))
            else:
                includePath.append(_make_path_relative(os.getcwd(), i))
        config_obj['includePath'] = includePath

        json_obj = {}
        json_obj['configurations'] = [config_obj]

        vsc_file.write(json.dumps(json_obj, ensure_ascii=False, indent=4))

    """
    Generate vscode.code-workspace files by build/compile_commands.json
    """
    if os.path.exists('build/compile_commands.json'):

        command_json_to_workspace(env['RTT_ROOT'],'build/compile_commands.json')
        return
    """
    Generate vscode.code-workspace files
    """
    with open('vscode.code-workspace', 'w') as vsc_space_file:
        info = utils.ProjectInfo(env)
        path_list = []
        for i in info['CPPPATH']:
            if  _make_path_relative(os.getcwd(), i)[0] == '.':
                if i[0] == '\"' and i[len(i) - 2:len(i)] == '\",':
                    path_list.append({'path':_make_path_relative(os.getcwd(), i[1:len(i) - 2])})
                else:
                    path_list.append({'path':_make_path_relative(os.getcwd(), i)})
        for i in info['DIRS']:
            if  _make_path_relative(os.getcwd(), i)[0] == '.':
                if i[0] == '\"' and i[len(i) - 2:len(i)] == '\",':
                    path_list.append({'path':_make_path_relative(os.getcwd(), i[1:len(i) - 2])})
                else:
                    path_list.append({'path':_make_path_relative(os.getcwd(), i)})

        json_obj = {}
        path_list = delete_repeatelist(path_list)
        path_list = sorted(path_list, key=lambda x: x["path"])
        for path in path_list:
            if path['path'] != '.':
                normalized_path = path['path'].replace('\\', os.path.sep)
                segments = [p for p in normalized_path.split(os.path.sep) if p != '..']
                path['name'] = 'rtthread/' + '/'.join(segments)
        json_obj['folders'] = path_list
        if os.path.exists('build/compile_commands.json'):
            json_obj['settings'] = {
            "clangd.arguments": [
                "--compile-commands-dir=.",
                "--header-insertion=never"
            ]
            }
        vsc_space_file.write(json.dumps(json_obj, ensure_ascii=False, indent=4))

    return

def GenerateProjectFiles(env):
    """
    Generate project.json file
    """
    if not os.path.exists('.vscode'):
        os.mkdir('.vscode')

    project = env['project']
    with open('.vscode/project.json', 'w') as vsc_file:
        groups = []
        for group in project:
            if len(group['src']) > 0:
                item = {}
                item['name'] = group['name']
                item['path'] = _make_path_relative(os.getcwd(), group['path'])
                item['files'] = []

                for fn in group['src']:
                    item['files'].append(str(fn))

                # append SConscript if exist
                if os.path.exists(os.path.join(item['path'], 'SConscript')):
                    item['files'].append(os.path.join(item['path'], 'SConscript'))

                groups.append(item)

        json_dict = {}
        json_dict['RT-Thread'] = env['RTT_ROOT']
        json_dict['Groups'] = groups

        # write groups to project.json
        vsc_file.write(json.dumps(json_dict, ensure_ascii=False, indent=4))

    return

def GenerateVSCode(env):
    print('Update setting files for VSCode...')

    GenerateProjectFiles(env)
    GenerateCFiles(env)
    print('Done!')

    return

import os

def find_rtconfig_dirs(bsp_dir, project_dir):
    """
    Search for subdirectories containing 'rtconfig.h' under 'bsp_dir' (up to 4 levels deep), excluding 'project_dir'.

    Args:
        bsp_dir (str): The root directory to search (absolute path).
        project_dir (str): The subdirectory to exclude from the search (absolute path).

    Returns
        list: A list of absolute paths to subdirectories containing 'rtconfig.h'.
    """

    result = []
    project_dir = os.path.normpath(project_dir)

    # list the bsp_dir to add result
    list = os.listdir(bsp_dir)
    for item in list:
        item = os.path.join(bsp_dir, item)

        # if item is a directory
        if not os.path.isdir(item):
            continue

        # print(item, project_dir)
        if not project_dir.startswith(item):
            result.append(os.path.abspath(item))

    parent_dir = os.path.dirname(project_dir)
    
    if parent_dir != bsp_dir:
        list = os.listdir(parent_dir)
        for item in list:
            item = os.path.join(parent_dir, item)
            rtconfig_path = os.path.join(item, 'rtconfig.h')
            if os.path.isfile(rtconfig_path):
                abs_path = os.path.abspath(item)
                if abs_path != project_dir:
                    result.append(abs_path)

    # print(result)
    return result

def GenerateVSCodeWorkspace(env):
    """
    Generate vscode.code files
    """
    print('Update workspace files for VSCode...')

    # get the launch directory
    cwd = GetLaunchDir()

    # get .vscode/workspace.json file
    workspace_file = os.path.join(cwd, '.vscode', 'workspace.json')
    if not os.path.exists(workspace_file):
        print('Workspace file not found, skip generating.')
        return

    try:
        # read the workspace file
        with open(workspace_file, 'r') as f:
            workspace_data = json.load(f)
        
        # get the bsp directories from the workspace data, bsps/folder
        bsp_dir = os.path.join(cwd, workspace_data.get('bsps', {}).get('folder', ''))
        if not bsp_dir:
            print('No BSP directories found in the workspace file, skip generating.')
            return
    except Exception as e:
        print('Error reading workspace file, skip generating.')
        return

    # check if .vscode folder exists, if not, create it
    if not os.path.exists(os.path.join(cwd, '.vscode')):
        os.mkdir(os.path.join(cwd, '.vscode'))

    with open(os.path.join(cwd, '.vscode/c_cpp_properties.json'), 'w') as vsc_file:
        info = utils.ProjectInfo(env)

        cc = os.path.join(rtconfig.EXEC_PATH, rtconfig.CC)
        cc = os.path.abspath(cc).replace('\\', '/')

        config_obj = {}
        config_obj['name'] = 'Linux'
        config_obj['defines'] = info['CPPDEFINES']

        intelliSenseMode = 'linux-gcc-arm'
        if cc.find('aarch64') != -1:
            intelliSenseMode = 'linux-gcc-arm64'
        elif cc.find('arm') != -1:
            intelliSenseMode = 'linux-gcc-arm'
        config_obj['intelliSenseMode'] = intelliSenseMode
        config_obj['compilerPath'] = cc
        config_obj['cStandard'] = "c99"
        config_obj['cppStandard'] = "c++11"

        # format "a/b," to a/b. remove first quotation mark("),and remove end (",)
        includePath = []
        for i in info['CPPPATH']:
            if i[0] == '\"' and i[len(i) - 2:len(i)] == '\",':
                includePath.append(_make_path_relative(cwd, i[1:len(i) - 2]))
            else:
                includePath.append(_make_path_relative(cwd, i))
        # make sort for includePath
        includePath = sorted(includePath, key=lambda x: x.lower())
        config_obj['includePath'] = includePath

        json_obj = {}
        json_obj['configurations'] = [config_obj]

        vsc_file.write(json.dumps(json_obj, ensure_ascii=False, indent=4))

    # generate .vscode/settings.json
    vsc_settings = {}
    settings_path = os.path.join(cwd, '.vscode/settings.json')
    if os.path.exists(settings_path):
        with open(settings_path, 'r') as f:
            # read the existing settings file and load to vsc_settings
            vsc_settings = json.load(f)

    with open(settings_path, 'w') as vsc_file:
        vsc_settings['files.exclude'] = {
            "**/__pycache__": True,
            "tools/kconfig-frontends": True,
        }

        result = find_rtconfig_dirs(bsp_dir, os.getcwd())
        if result:
            # sort the result
            result = sorted(result, key=lambda x: x.lower())
            for item in result:
                # make the path relative to the current working directory
                rel_path = os.path.relpath(item, cwd)
                # add the path to files.exclude
                vsc_settings['files.exclude'][rel_path] = True

        vsc_settings['search.exclude'] = vsc_settings['files.exclude']
        # write the settings to the file
        vsc_file.write(json.dumps(vsc_settings, ensure_ascii=False, indent=4))

    print('Done!')

    return
