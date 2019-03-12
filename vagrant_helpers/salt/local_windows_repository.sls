---
# Salt state for using a local source for Windows software definitions
# - - - this state is to run on the Salt Master
#
{% if grains['os'] == 'Windows' %}
c:/srv/salt/win/repo-ng:
{%  else %}
/srv/salt/win/repo-ng:
{% endif %}
  file.directory:
    - makedirs: true

{# -- NOTE -- no jinja is used here. The jinja template is expanded on the minion only. #}
# Sample: use a local definition to find Notepad++
/srv/salt/win/repo-ng/npp.sls:
  file.managed:
    - source: salt://{{ slspath }}/files/npp.sls.source

/srv/salt/win/repo-ng/git.sls:
  file.managed:
    - source: salt://{{ slspath }}/files/git.sls.source

/srv/salt/win/repo-ng/VCforPython27.sls:
  file.managed:
    - source: salt://{{ slspath }}/files/VCforPython27.sls.source
...
