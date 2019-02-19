{
  "targets": [
    {
      "target_name": "cut",
      "sources": [ "cut.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "OpenMesh/src/"
      ],
      "libraries": [ "../OpenMesh/build/src/OpenMesh/Core/Release/OpenMeshCore.lib"],
       'cflags_cc!': [
        '-fno-rtti'
      ],
      'conditions': 
      [
        [
          'OS=="mac"',
          {
            'xcode_settings': { 'GCC_ENABLE_CPP_RTTI': 'YES' }
          },
        ],
        [
          'OS=="win"',
          {                        
            'cflags': [
                '/GR',
              ],
            'configurations': {
              'Debug': {
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'RuntimeTypeInfo': 'true',
                    'AdditionalOptions': ['/GR', '/MT'],
                  }
                }
              },
              'Release': {                            
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'RuntimeTypeInfo': 'true',
                    'AdditionalOptions': ['/GR', '/MT'],
                  }
                }
              }
            }
          }
        ]
      ],
    }
  ]
}