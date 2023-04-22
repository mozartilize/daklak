project(
    'daklak',
    'c',
    version: '0.1.0',
    meson_version: '>=0.59.0',
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
wayland_protocols_dep = dependency('wayland-protocols')
xkbcommon_dep = dependency('xkbcommon')
pthread_dep = dependency('threads')
appindicator_dep = dependency('appindicator3-0.1')
scfg_dep = dependency('scfg', fallback: 'libscfg')

daklakwl_src = files(
    'daklakwl.c',
    'actions.c',
    'buffer.c',
    'config.c',
    'tray.c',
)
daklakwl_inc = []

subdir('buildtools')
subdir('protocol')
subdir('include')

daklak_bin = executable(
    'daklak',
    daklakwl_src,
    install: true,
    include_directories: daklakwl_inc,
    dependencies: [
        wayland_client_dep,
        xkbcommon_dep,
        protocols_dep,
        pthread_dep,
        appindicator_dep,
        scfg_dep,
    ],
)
