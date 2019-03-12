# file /bevy_srv/pillar/top.sls
#
#   This is an actual pillar top file (not a template)
#
#   In practice, this file will only be used by "salt-call --local" runs.
#   /srv/pillar/top.sls would be the real thing in normal Salt Master operation.
#
# Masterless minions are expected to be run with an /etc/salt/minion configuration like:
# pillar_root:
#   - /srv/pillar
#   - /vagrant/vagrant_helpers/pillar
#
# make local modifications in /srv/pillar/???
#
base:
  '*':
    - core_settings  # all systems share these
