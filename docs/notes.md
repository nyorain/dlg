Notes
=====

# General
 
- The default of DLG_FILE (stripping DLG_BASE_PATH (if existent) from the current file)
  will not really compare DLG_BASE_PATH with the file name but just skip its length

TODO: Add some examples on how tags could be used

# Windows/msvc troubleshooting

Windows and msvc are fully supported (and tested) by dlg.
We try to work around most of the... issues on windows by there are a few
things you have to keep in mind for dlg to work.

- msvc (at the state of 2017, still) does not allow/correctly handle utf-8 string literals
	- this means ```dlg_fprintf(stdout, u8"äü")``` will not work by default since the
	  string passed there is not utf-8 encoded
	- The above example can be made to work using the ```/utf-8``` switch on msvc
- note that dlg always passes filepaths in their native representation so with msvc
  the filepath will have backslashes (important if you e.g. want to handle logging
  calls from different files differently)
- meson on windows: if you want to define DLG_BASE_PATH using meson you will have
  to work around the backslashes (e.g. in the path returned from ```meson.source_root()```)
  sine those would be interpreted as invalid escape characters and there is not meson
  function to escape backslashes correctly (as of 2017).
  
  DLG itself handles it this way:
  ```meson
source_root = meson.source_root().split('\\')
add_project_arguments('-DDLG_BASE_PATH="' + '/'.join(source_root) + '/"', language: 'c')
  ```