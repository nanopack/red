[![red logo](http://nano-assets.gopagoda.io/readme-headers/red.png)](http://nanobox.io/open-source#red)
 [![Build Status](https://travis-ci.org/nanopack/red.svg)](https://travis-ci.org/nanopack/red)
# red
A fast, in-kernel, ad-hoc point-to-point vxlan network. This project interacts with [redd](https://github.com/nanopack/redd) to build virtual networks. The redd project is the daemon that interacts with the kernel's vxlan module. This project is the command-line utility used to interact with redd.

## How to use red:

### Usage
    Usage: red [OPTIONS] <subcommand>  <args> ...
        -h <hostname>      red hostname (default: 127.0.0.1)
        -p <port>          red port (default: 4470)
        --help             Output this help and exit
        --version          Output version and exit
        --yaml             Format output in YAML

        add-ip <ip address>
        remove-ip <ip address>
        show-ip

        add-node <ip address | hostname>
        remove-node <ip address | hostname>
        show-node

        ping

        status

### Options
#### -h <hostname>
Specify the host to connect to
#### -p <port>
Specify the port to connect to
#### --help
Show the help for red
#### --version
Show the version number of red
#### --yaml
Format the output of subcommands to be yaml
### Subcommands
#### add-ip
Add ip to the vxlan interface. Must specify with the cidr (192.168.0.1/24).
#### remove-ip
Remove ip from the vxlan interface. Must specify with the cidr (192.168.0.1/24).
#### show-ip
Show a list of IPs attached to the vxlan interface.
#### add-node
Add host node to peer list.
#### remove-node
Remove host node from peer list.
#### show-node
Show a list of nodes in the peer list.
#### ping
Sends a simple request to the redd api to see if it is listening.
#### status
Shows information about the redd server, including ips and nodes.

[![open source](http://nano-assets.gopagoda.io/open-src/nanobox-open-src.png)](http://nanobox.io/open-source)