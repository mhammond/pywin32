---
# salt state file for all systems
{# this is an example of things you may always want installed. #}

{% if grains['os_family'] == 'Windows' %}
pkg.refresh_db:
  module.run:
  - require_in:
    - pkg: windows_packages
windows_packages:
{# Assumes that you ran salt_master.local_windows_repository on the Master #}
  pkg.installed:
    - pkgs:
      - npp
      - git

choco_boot:
  cmd.run:
    - name: salt-call chocolatey.bootstrap
    - require_in:
      - windows_py3

windows_py3:
  chocolatey.installed:
    - name: python3

windows_pygit2_failure_workaround:
   cmd.run:
     - name: c:\salt\bin\python -m pip install pygit2

{# Note: .sls files are interpreted on the Minion, so the environment variables are local to it #}
{{ salt['environ.get']('SystemRoot') }}/edit.bat:  {# very dirty way to create an "edit" command for all users #}
  file.managed:
    - contents:
      - '"{{ salt['environ.get']('ProgramFiles(x86)') }}\Notepad++\Notepad++.exe" %*'
    - unless:  {# do not install this if there is an existing "edit" command #}
      - where edit

    {{ salt['environ.get']('SystemRoot') }}/tail.bat:  {# very dirty way to create a "tail -f" command for all users #}
  file.managed:
    - contents: |
        @ECHO OFF
        IF "%1"=="-f" (
        powershell get-content "%2" -tail 20 -wait
        ) ELSE (
        start /b powershell get-content "%1" -tail 20
        )
    - unless:  {# do not install this if there is an existing "tail" command #}
      - where tail

restart-the-minion:
  cmd.run:
    - bg: true  # do not wait for completion of this command
    - order: last
    - name: 'C:\salt\salt-call.bat service.restart salt-minion'
{% endif %}  {# Windows #}

{% if grains['os_family'] == 'Debian' %}
debian_packages:
  pkg.installed:
    - pkgs:
      - git
      - nano
      - python-pip
      - python3
      - python3-pip
      - tree
{% endif %}

{% if salt['grains.get']('os') == 'Ubuntu' %}
ubuntu_packages:
  pkg.installed:
    - pkgs:
      {% if grains['osrelease'] < '18.04' %}
      - python-software-properties
      {% endif %}
      - vim-tiny
      - virt-what
      {% if grains['osrelease'] < '16.04' %}
      - python-git  # fallback package if pygit2 is not found.
      {% else %}
      - python-pygit2
      {% endif %}
      {% if grains['locale_info']['defaultlanguage'] != 'en_US' %}
      - 'language-pack-en'
      {% endif %}
{% endif %}
...
