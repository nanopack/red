# VTEP

## What is VTEP?
Vtep is the command line cli to interact with the vtepd daemon.

## How to use VTEP:

### Usage
    Usage: vtep [OPTIONS] <subcommand>  <args> ...
        -h <hostname>      VTEP hostname (default: %s)
        -p <port>          VTEP port (default: %i)
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
Show the help for vtep
#### --version
Show the version number of vtep
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
Sends a simple request to the vtepd api to see if it is listening.
#### status
Shows information about the vtepd server, including ips and nodes.