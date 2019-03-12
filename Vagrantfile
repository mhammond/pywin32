# -*- mode: ruby -*-
# vi: set ft=ruby :
#  .  .  .  .  NOTE  .  .  .  .
# This configuration file is written in Ruby.
# I invested one entire day in learning Ruby,
# so if this is not particularly good Ruby code, I'm sorry.
# -- vernondcole 2019 .  .  .  .
require "etc"
require "ipaddr"

vagrant_command = ARGV[0]
vagrant_object = ARGV.length > 1 ? ARGV[1] : ""  # the name (if any) of the vagrant VM for this command
#
  settings = {"bevy" => "local",
  "vagrant_prefix" => '172.17',
  "vagrant_interface_guess" => "eth0",
   "master_vagrant_ip" => 'localhost',
   "my_windows_user" => 'vagrant',
   "my_windows_password" => 'vagrant',
   "fqdn_pattern" => '{}.{}.test',
   "force_linux_user_password" => false,
   "WINDOWS_GUEST_CONFIG_FILE" => 'configure_machine/masterless_minion.conf',
   }
  default_run_highstate = false

# .
BEVY = settings["bevy"]  # the name of your bevy
# the first two bytes of your Vagrant host-only network IP such as ("192.168.x.x")
NETWORK = "#{settings['vagrant_prefix']}"
# ^ ^ each VM below will have a NAT network in NETWORK.17.x/27.
puts "Your bevy name:#{BEVY} using local network #{NETWORK}.x.x"
puts "This (the VM host) computer will be at #{NETWORK}.2.1" if ARGV[1] == "up"
bevy_mac = (BEVY.to_i(36) % 0x1000000).to_s(16)  # a MAC address based on hash of BEVY
# in Python that would be: bevy_mac = format(int(BEVY, base=36) % 0x1000000, 'x')
#
VAGRANT_HOST_NAME = Socket.gethostname
login = Etc.getlogin    # get your own user information
my_linux_user = login if my_linux_user.to_s.empty?  # use current value if settings gives blank.
#
# . v . v . the program starts here . v . v . v . v . v . v . v . v . v .
#
# Bridged networks make the machine appear as another physical device on your network.
# We must supply a list of names to avoid Vagrant asking for interactive input
#
if (RUBY_PLATFORM=~/darwin/i)  # on Mac OS, guess some frequently used ports
  interface_guesses = ['en0: Ethernet', 'en1: Wi-Fi (AirPort)',  'en0: Wi-Fi (Wireless)']
else  # Windows or Linux
  interface_guesses = settings['vagrant_interface_guess']
end
if vagrant_command == "up" or vagrant_command == "reload"
  puts "Running on host #{VAGRANT_HOST_NAME}"
  puts "Will try bridge network using interface(s): #{interface_guesses}"
end

max_cpus = Etc.nprocessors / 2 - 1
max_cpus = 1 if max_cpus < 1

Vagrant.configure(2) do |config|  # the literal "2" is required.

  config.ssh.forward_agent = true

  # . . . . . . . . . . . . Define machine test1 . . . . . . . . . . . . . .
  # This is the default machine. It has no Salt provisioning.
  #
  config.vm.define "test1", primary: true do |quail_config|  # this will be the default machine for "vagrant up"
    quail_config.vm.box = "StefanScherer/windows_10"
    # <#this causes Windows to restart#> # quail_config.vm.hostname = 'test1'
    quail_config.vm.network "private_network", ip: NETWORK + ".2.201"  # needed so saltify_profiles.conf can find this unit
    if vagrant_command == "up" and (ARGV.length == 1 or (vagrant_object == "test1"))
      puts "Starting 'test1' at #{NETWORK}.2.201..."
    end
    quail_config.vm.network "public_network", bridge: interface_guesses
    quail_config.vm.provider "virtualbox" do |v|  # only for VirtualBox boxes
        v.name = BEVY + '_test1'  # ! N.O.T.E.: name must be unique
        v.gui = true  # turn on the graphic window
        v.customize ["modifyvm", :id, "--vram", "33"]  # enough video memory for full screen
        v.memory = 4096
        v.cpus = max_cpus
        v.linked_clone = true # make a soft copy of the base Vagrant box
        v.customize ["modifyvm", :id, "--natnet1", NETWORK + ".17.0/27"]  # do not use 10.0 network for NAT
	    #                                                     ^  ^/27 is the smallest network allowed.
        v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]  # use host's DNS resolver
        v.customize ["storageattach", :id, "--storagectl", "IDE Controller", "--port", "1", "--device", "0", "--type", "dvddrive", "--medium", "emptydrive"]
    end
    quail_config.vm.guest = :windows
    quail_config.vm.boot_timeout = 900
    quail_config.vm.graceful_halt_timeout = 90
  end
  # . . . . . . . . . . . . Define machine win10 . . . . . . . . . . . . . .
  # . this Windows 10 machine bootstraps Salt.
  config.vm.define "win10", autostart: false do |quail_config|
    quail_config.vm.box = "StefanScherer/windows_10"  #"Microsoft/EdgeOnWindows10"
    # <#this causes Windows to restart#> # quail_config.vm.hostname = 'win10'
    quail_config.vm.network "public_network", bridge: interface_guesses
    quail_config.vm.network "private_network", ip: NETWORK + ".2.10"
    if vagrant_command == "up" and vagrant_object == "win10"
      puts "Starting #{vagrant_object} as a Salt minion of #{settings['master_vagrant_ip']}."
      puts ""
      puts "NOTE: you may need to run \"vagrant up\" twice for this Windows minion."
      puts ""
      end
    quail_config.vm.provider "virtualbox" do |v|
        v.name = BEVY + '_win10'  # ! N.O.T.E.: name must be unique
        v.gui = true  # turn on the graphic window
        v.linked_clone = true
        v.customize ["modifyvm", :id, "--vram", "33"]  # enough video memory for full screen
        v.memory = 4096
        v.cpus = max_cpus
        v.customize ["modifyvm", :id, "--natnet1", NETWORK + ".17.192/27"]  # do not use 10.0 network for NAT
        v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]  # use host's DNS resolver
        v.customize ["storageattach", :id, "--storagectl", "IDE Controller", "--port", "1", "--device", "0", "--type", "dvddrive", "--medium", "emptydrive"]
    end
    quail_config.vm.guest = :windows
    quail_config.vm.boot_timeout = 900
    quail_config.vm.graceful_halt_timeout = 90
    script = "new-item C:\\salt\\conf\\minion.d -itemtype directory -ErrorAction silentlycontinue\r\n"
    quail_config.vm.provision "shell", inline: script
    if settings.has_key?('WINDOWS_GUEST_CONFIG_FILE') and File.exist?(settings['WINDOWS_GUEST_CONFIG_FILE'])
      quail_config.vm.provision "file", source: settings['WINDOWS_GUEST_CONFIG_FILE'], destination: "c:\\salt\\conf\\minion.d\\00_vagrant_boot.conf"
      end
    quail_config.vm.provision :salt do |salt|  # salt_cloud cannot push Windows salt
        salt.minion_id = "win10"
        salt.master_id = "#{settings['master_vagrant_ip']}"
        #salt.log_level = "info"
        salt.verbose = false
        salt.colorize = true
        salt.run_highstate = default_run_highstate
    end
  end

 # . . . . . . . . . . . . Define machine win16 . . . . . . . . . . . . . .
 # . this machine installs Salt on a Windows 2016 Server.
  config.vm.define "win16", autostart: false do |quail_config|
    quail_config.vm.box = "cdaf/WindowsServer" #gusztavvargadr/w16s" # Windows Server 2016 standard
    quail_config.vm.network "public_network", bridge: interface_guesses
    quail_config.vm.network "private_network", ip: NETWORK + ".2.16"
    if vagrant_command == "up" and vagrant_object == "win16"
      puts "Starting #{vagrant_object} as a Salt minion of #{settings['master_vagrant_ip']}."
      end
    quail_config.vm.provider "virtualbox" do |v|
        v.name = BEVY + '_win16'  # ! N.O.T.E.: name must be unique
        v.gui = true  # turn on the graphic window
        v.linked_clone = true
        v.customize ["modifyvm", :id, "--vram", "27"]  # enough video memory for full screen
        v.memory = 4096
        v.cpus = max_cpus
        v.customize ["modifyvm", :id, "--natnet1", NETWORK + ".17.224/27"]  # do not use 10.0 network for NAT
        v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]  # use host's DNS resolver
    end
    quail_config.vm.guest = :windows
    quail_config.vm.boot_timeout = 300
    quail_config.vm.graceful_halt_timeout = 60
    quail_config.vm.communicator = "winrm"
    script = "new-item C:\\salt\\conf\\minion.d -itemtype directory\r\n" # -ErrorAction silentlycontinue\r\n"
    script += "route add 10.0.0.0 mask 255.0.0.0 #{NETWORK}.17.226 -p\r\n"  # route 10. network through host NAT for VPN
    quail_config.vm.provision "shell", inline: script
    if settings.has_key?('WINDOWS_GUEST_CONFIG_FILE') and File.exist?(settings['WINDOWS_GUEST_CONFIG_FILE'])
      quail_config.vm.provision "file", source: settings['WINDOWS_GUEST_CONFIG_FILE'], destination: "c:\\salt\\conf\\minion.d\\00_vagrant_boot.conf"
      end
    quail_config.vm.provision :salt do |salt|
        salt.minion_id = "win16"
        salt.master_id = "#{settings['master_vagrant_ip']}"
        salt.log_level = "info"
        salt.verbose = true
        salt.colorize = true
        salt.run_highstate = default_run_highstate
    end
  end

   # . . . . . . . . . . . . Define machine win19 . . . . . . . . . . . . . .
   # . this machine installs Salt on a Windows 2019 Server.
    config.vm.define "win19", autostart: false do |quail_config|
      quail_config.vm.box = "StefanScherer/windows_2019"
      quail_config.vm.network "public_network", bridge: interface_guesses
      quail_config.vm.network "private_network", ip: NETWORK + ".2.19"
      if vagrant_command == "up" and vagrant_object == "win19"
        puts "Starting #{vagrant_object} as a Salt minion of #{settings['master_vagrant_ip']}."
        puts "NOTE: you may need to hit <Ctrl C> after starting this Windows minion."
        end
      quail_config.vm.provider "virtualbox" do |v|
          v.name = BEVY + '_win19'  # ! N.O.T.E.: name must be unique
          v.gui = true  # turn on the graphic window
          v.linked_clone = true
          v.customize ["modifyvm", :id, "--vram", "27"]  # enough video memory for full screen
          v.memory = 4096
          v.cpus = max_cpus
          v.customize ["modifyvm", :id, "--natnet1", NETWORK + ".17.32/27"]  # do not use 10.0 network for NAT
          v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]  # use host's DNS resolver
      end
      quail_config.vm.guest = :windows
      quail_config.vm.boot_timeout = 300
      quail_config.vm.graceful_halt_timeout = 60
      if settings.has_key?('WINDOWS_GUEST_CONFIG_FILE') and File.exist?(settings['WINDOWS_GUEST_CONFIG_FILE'])
        quail_config.vm.provision "file", source: settings['WINDOWS_GUEST_CONFIG_FILE'], destination: "c:\\salt\\conf\\minion.d\\00_vagrant_boot.conf"
        end
      quail_config.vm.provision :salt do |salt|
          salt.minion_id = "win19"
          salt.master_id = "#{settings['master_vagrant_ip']}"
          salt.log_level = "info"
          salt.verbose = true
          salt.colorize = true
          salt.run_highstate = false  # Vagrant may stall trying to run Highstate for this minion.
      end
    end
end
