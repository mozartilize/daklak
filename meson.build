project(
    'daklak',
    'c',
    version: '0.1.0',
    default_options: [
        'c_std=c11',
        'warning_level=2',
    ]
)

add_project_arguments(
    [
        '-D_GNU_SOURCE',
        '-Wno-unused-parameter',
        '-Wno-missing-field-initializers',
    ],
    language: 'c',
)

cc = meson.get_compiler('c')

rt_dep = cc.find_library('rt')
wayland_client_dep = dependency('wayland-client')
wayland_protocols_dep = dependency('wayland-protocols')
xkbcommon_dep = dependency('xkbcommon')
pthread_dep = dependency('threads')
appindicator_dep = dependency('appindicator3-0.1')

subdir('protocol')

daklak_bin = executable(
    'daklak',
    'daklakwl.c',
    'buffer.c',
    'tray.c',
    dependencies: [
        wayland_client_dep,
        xkbcommon_dep,
        protocols_dep,
        pthread_dep,
        appindicator_dep,
    ],
)
