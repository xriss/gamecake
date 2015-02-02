# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'component%': 'static_library',
        'angle_path%': '..',
        'windows_sdk_path%': 'C:/Program Files (x86)/Windows Kits/8.1',
        'angle_build_winrt%': '0',
        'angle_build_winphone%': '0',
    },
    'msvs_disabled_warnings': [ 4075, 4100, 4127, 4239, 4244, 4245, 4251, 4264, 4267, 4447, 4512, 4530, 4702, 4718 ],
    'msvs_system_include_dirs':
    [
        '<(windows_sdk_path)/Include/shared',
        '<(windows_sdk_path)/Include/um',
    ],
    'conditions':
    [
        ['component=="shared_library"',
        {
            'defines': [ 'COMPONENT_BUILD' ],
        }],
    ],
    'msvs_settings':
    {
        'VCCLCompilerTool':
        {
            'PreprocessorDefinitions':
            [
                '_CRT_SECURE_NO_DEPRECATE',
                '_SCL_SECURE_NO_WARNINGS',
                'NOMINMAX',
            ],
        },
        'VCLinkerTool':
        {
            'conditions':
            [
                ['angle_build_winrt==0',
                {
                    'AdditionalDependencies':
                    [
                        'kernel32.lib',
                        'gdi32.lib',
                        'winspool.lib',
                        'comdlg32.lib',
                        'advapi32.lib',
                        'shell32.lib',
                        'ole32.lib',
                        'oleaut32.lib',
                        'user32.lib',
                        'uuid.lib',
                        'odbc32.lib',
                        'odbccp32.lib',
                        'delayimp.lib',
                    ],
                }],
                # winrt compilation is dynamic depending on the project
                # type. AdditionalDependencies is automatically configured
                # with the required .libs
                ['angle_build_winrt==1',
                {
                    'AdditionalDependencies':
                    [
                        '%(AdditionalDependencies)',
                        'uuid.lib',
                    ],
                }],
            ],
        },
    },

    # Windows SDK library directories for the configurations
    'configurations':
    {
        'x86_Base':
        {
            'msvs_settings':
            {
                'VCLinkerTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/winv6.3/um/x86',
                    ],
                },
                'VCLibrarianTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/winv6.3/um/x86',
                    ],
                },
            },
        },
        'x64_Base':
        {
            'msvs_settings':
            {
                'VCLinkerTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/winv6.3/um/x64',
                    ],
                },
                'VCLibrarianTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/winv6.3/um/x64',
                    ],
                },
            },
        },
        'conditions':
        [
            ['angle_build_winrt==1',
            {
                'arm_Base':
                {
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'AdditionalLibraryDirectories':
                            [
                                '<(windows_sdk_path)/Lib/winv6.3/um/arm',
                            ],
                        },
                        'VCLibrarianTool':
                        {
                            'AdditionalLibraryDirectories':
                            [
                                '<(windows_sdk_path)/Lib/winv6.3/um/arm',
                            ],
                        },
                    },
                },
            }],
        ],
    },
}
