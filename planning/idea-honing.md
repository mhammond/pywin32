### INSTRUCTIONS: DO NOT MODIFY OR DELETE ###
Read rough-idea.md. Ask me one question at a time so we can develop a thorough, step-by-step spec for this idea. Each question should build on my previous answers, and our end goal is to have a detailed specification I can hand off to a developer. Let's do this iteratively and dig into every relevant detail. Remember, only one question at a time and don't repeat questions I've already answered. Update idea-honing.md as we go to track our progress.
### END INSTRUCTIONS ###
---

# Long Path Support for pythonservice.exe

## Current Understanding

Based on the planning documents, I understand that:

1. Python is long path aware by default, but pythonservice.exe in pywin32 is not
2. This causes issues when running Python applications as Windows services that need to access long paths
3. Long path support can be enabled through an application manifest with the longPathAware element
4. The Windows registry must also have long path support enabled (LongPathsEnabled=1)
5. The solution should be implemented at build time, not as a post-build modification

## Idea Honing Process

### Question 1: What is the specific build process change needed to add the application manifest to pythonservice.exe?

Modify existing code to support this. Do before build time. 

### Question 2: Should the manifest include any other Windows features besides long path support? 
no

### Question 3: How should we test that long path support is working correctly in pythonservice.exe?



### Question 4: Are there any backward compatibility concerns with adding long path support?

Yes, this should be a new feature that doesn't affect past versions

### Question 5: Should we document the long path support feature in the pywin32 documentation?

Yes 
### Question 6: Are there any other pywin32 executables that should also be made long path aware?

No

### Question 7: How should we handle the case where the Windows registry setting for long paths is not enabled?

if it is not enabled do not enable Long path support

### Question 8: What specific Windows API functions used by pythonservice.exe will benefit from long path support?

Not needed


### Question 9: Should we add any Python helper functions to check if long path support is available?

no you can just check in the code



### Question 10: What version of pywin32 should include this change?

The latest
