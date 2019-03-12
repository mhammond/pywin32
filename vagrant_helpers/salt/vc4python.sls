---
# salt state file to install the Visual C++ compiler used for Python versions 2.6 thru 3.2

{% if grains['os_family'] == 'Windows' %}
pkg.refresh_db_p27:
  module.run:
    - name: pkg.refresh_db
    - require_in:
      - pkg: VCforPython27

VCforPython27:
{# Assumes that you ran salt_master.local_windows_repository on the Master #}
  pkg.installed

{% else %}  {# Not Windows #}

VCforPython27:
  test.fail_without_changes:
    - failhard: True
    - name: We do not know how to install Visual C++ 9.0 without Windows
{% endif %}
...
