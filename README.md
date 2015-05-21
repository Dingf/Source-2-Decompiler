# Source 2 Decompiler (S2DC)
It decompiles Source 2 things.
## Usage
    Usage: s2dc [-d <input path>] [-o <output path>] <input(s)>
    
      [Input]
          -d    Specifies the path to search for input files.
                If an absolute pathname is given, this path is ignored.
      [Output]
          -o    Specifies the output path. This path only serves as the
                root folder for the output. The directory structure
                relative to the input path is still preserved.
                
S2DC searches the input path (or dota_ugc\\game\\dota_imported, if not specified) for all files that match the input file(s). Wildcards in the input (*) are accepted, and will correspond to any file which also matches the pattern. Currently supports the .vtex_c, .vmat_c, and .vpcf_c file formats.


Decompiled resources will be written to the output path (or the local directory, if not specified), but their directory structure relative to the input path is still preserved. If the input is an absolute path, the output will be found in the root folder (the output path) instead.
