---
# Salt state for installing Visual Studio 2017 Professional
#
include:
  - windows.install_chocolatey

visualstudio2017community:
  chocolatey.installed:
    - name: visualstudio2017community

visualstudio2017-workload-vctool:
  chocolatey.installed:
    - name: visualstudio2017-workload-vctools
    - require:
      - visualstudio2017community
...
