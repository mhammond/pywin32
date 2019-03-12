---
# Salt state for installing the chocolatey installation tool
#
{# using cmd.run rather than module.run to get an extra layer of isolation #}
install_chocolatey:
  cmd.run:
    - name: salt-call chocolatey.bootstrap

{# NOTE: there is a bug in old Salt-Minions. You may need to hand install 2018.3+ for chocolatey to work.  #}
...
