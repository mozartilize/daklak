project(
    'daklak',
    'c',
    default_options: [
        'c_std=c11',
        'warning_level=2',
    ]
)

add_project_arguments(
    [
        '-D_GNU_SOURCE',
        '-Wno-unused-parameter',
    ],
    language: 'c',
)

cc = meson.get_compiler('c')

rt_dep = cc.find_library('rt')
wayland_client_dep = dependency('wayland-client')
wayland_cursor_dep = dependency('wayland-cursor')
wayland_protocols_dep = dependency('wayland-protocols')
xkbcommon_dep = dependency('xkbcommon')

subdir('protocol')

daklak_bin = executable(
    'daklak',
    'daklakwl.c',
    'buffer.c',
    dependencies: [
        wayland_client_dep,
        wayland_cursor_dep,
        xkbcommon_dep,
        protocols_dep,
    ],
)
