---
# salt pillar file for definitions common to all systems.

# text used to tell programmers about changing files written by SaltStack
salt_created_message: "## This file originally created by SaltStack, but if you alter it, it will NOT be replaced."
salt_managed_message: "## This file managed by SaltStack. Any changes may be overwritten."
salt_managed_directory: "## Some contents of this directory are managed by SaltStack. Changes to supplied files may be overwritten."

{% if grains['os_family'] == "Windows" %}
salt_config_directory: "C:/salt/conf"
{% else %}
salt_config_directory: "/etc/salt"
{% endif %}
...
