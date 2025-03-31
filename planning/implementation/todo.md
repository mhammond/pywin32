# Implementation Todo List: Long Path Support for PythonService.exe

This checklist tracks the implementation progress for adding long path support to pythonservice.exe, following the detailed design document.

## 6.1 Phase 1: Development

- [ ] Create the manifest file
  - [ ] Create directory for manifest if needed
  - [ ] Write pythonservice.exe.manifest with longPathAware element
  - [ ] Validate manifest XML syntax

- [ ] Modify the build process to embed the manifest
  - [ ] Locate appropriate build step in setup.py
  - [ ] Add code to generate/copy manifest file
  - [ ] Add mt.exe command to embed manifest
  - [ ] Handle potential errors if mt.exe is not available

- [ ] Build a test version of pywin32 with the changes
  - [ ] Verify manifest is correctly embedded
  - [ ] Check for any build warnings or errors

## 6.2 Phase 2: Testing

- [ ] Execute the test cases defined in the design document
  - [ ] Basic functionality test (normal paths)
  - [ ] Long path test with registry enabled
  - [ ] Long path test with registry disabled
  - [ ] Edge cases (paths near 260 chars, Unicode paths, network paths)

- [ ] Verify backward compatibility
  - [ ] Test on existing services
  - [ ] Verify no regression in normal path handling

- [ ] Test on different Windows versions
  - [ ] Windows 10 version 1607+
  - [ ] Windows 11
  - [ ] Verify graceful behavior on unsupported Windows versions

## 6.3 Phase 3: Documentation and Release

- [ ] Update documentation to mention long path support
  - [ ] Add note to README.md about long path support
  - [ ] Document registry requirement for users

- [ ] Add a note to CHANGES.txt
  - [ ] Describe the feature addition
  - [ ] Note any limitations or requirements

- [ ] Submit pull request
  - [ ] Create detailed PR description
  - [ ] Reference any related issues
  - [ ] Address review feedback

## Additional Tasks

- [ ] Consider adding a utility function to check if long paths are supported
- [ ] Explore adding long path support to other pywin32 executables if needed
- [ ] Create a simple test script that demonstrates the functionality
