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

Due to a change in Dota 2 Reborn, some files have had their file structure changed, and no longer contain any structure metadata. To decompile these files, I have provided external metadata, located in the ntro folder. Place the contents of this folder in the same directory as your S2DC executable (so if your executable is in C:\s2dc, then the folder should be named C:\s2dc\ntro). Currently required for decompiling .vtex_c and .vpcf_c files.
