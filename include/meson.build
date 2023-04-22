foreach file: ['default_config']
    daklakwl_src += custom_target(file,
        input: '../data' / file,
        output: file + '.inc',
        command: [file2string, '@INPUT@', '@OUTPUT@'],
    )
endforeach

daklakwl_inc += include_directories('.')
