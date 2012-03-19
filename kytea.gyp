{
  'variables': {
    'pkgdatadir': '',
    'include_dirs': [
      'src/include',
      'src/include/kytea',
    ],
  },
  'targets': [
    {
      'target_name': 'run_kytea',
      'type': 'executable',
      'include_dirs': [
        '<@(include_dirs)',
      ],
      'sources': [
        'src/bin/run-kytea.cpp',
      ],
      'dependencies': [
        'kytea',
      ],
      'conditions': [
        ['OS=="linux"', {
          'cflags': [
            '-fexceptions',
          ],
        }],
      ],
    }, {
      'target_name': 'train_kytea',
      'type': 'executable',
      'include_dirs': [
        '<@(include_dirs)',
      ],
      'sources': [
        'src/bin/train-kytea.cpp',
      ],
      'dependencies': [
        'kytea',
      ],
      'conditions': [
        ['OS=="linux"', {
          'cflags': [
            '-fexceptions',
          ],
        }],
      ],
    }, {
      'target_name': 'kytea',
      'type': 'static_library',
      'include_dirs': [
        '<@(include_dirs)',
      ],
      'sources': [
        'src/lib/corpus-io.cpp',
        'src/lib/dictionary.cpp',
        'src/lib/feature-io.cpp',
        'src/lib/feature-lookup.cpp',
        'src/lib/kytea-config.cpp',
        'src/lib/kytea-lm.cpp',
        'src/lib/kytea-model.cpp',
        'src/lib/kytea.cpp',
        'src/lib/model-io.cpp',
        'src/lib/string-util.cpp',
        'src/lib/liblinear/linear.cpp',
        'src/lib/liblinear/linear.h',
        'src/lib/liblinear/tron.cpp',
        'src/lib/liblinear/tron.h',
        'src/lib/liblinear/blas/blas.h',
        'src/lib/liblinear/blas/blasp.h',
        'src/lib/liblinear/blas/daxpy.c',
        'src/lib/liblinear/blas/ddot.c',
        'src/lib/liblinear/blas/dnrm2.c',
        'src/lib/liblinear/blas/dscal.c',
      ],
      'conditions': [
        ['OS=="linux"', {
          'cflags': [
            '-fexceptions',
          ],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',  # -fexceptions
          },
        }],
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'CompileAs': '0',
            },
          },
        }],
      ],
      'defines': [
        'PKGDATADIR="<(pkgdatadir)"',
      ],
    }],
}
