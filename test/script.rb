#!/usr/bin/env ruby

host = "red.local"
port = 4470
verbose = false

max_vxlans = 100
simnets = 1000
members = 1000

def create_random_mac
  (1..6).map{"%0.2x"%rand(256)}.join(":")
end

def create_random_host
  (1..4).map{"%d"%(rand(254) + 1)}.join(".")
end

vxlans = {}

simnets.times do |sim_num|
  vxlan = rand(max_vxlans) + 1
  simnet = "simnet#{sim_num}"
  mac = create_random_mac
  if vxlans[vxlan].nil?
    vxlans[vxlan] = {devices: [], members: []}
  end
  vxlans[vxlan][:devices] << {mac: mac, simnet: simnet}
  output = `src/vxadm -h #{host} -p #{port} add-device -v #{vxlan} -m #{mac} #{simnet}`
  puts "src/vxadm -h #{host} -p #{port} add-device -v #{vxlan} -m #{mac} #{simnet}" if verbose
  puts output if verbose
end

members.times do |num|
  vxlan = 0
  begin
    vxlan = rand(max_vxlans) + 1
  end while vxlans[vxlan].nil?
  vxlans[vxlan]
  mac = create_random_mac
  mem_host = create_random_host
  output = `src/vxadm -h #{host} -p #{port} add-member -v #{vxlan} -h #{mem_host} #{mac}`
  puts "src/vxadm -h #{host} -p #{port} add-member -v #{vxlan} -h #{mem_host} #{mac}" if verbose
  puts output if verbose
end

# simnets.times do |sim_num|
#   simnet = "simnet#{sim_num}"
#   output = `src/vxadm -h #{host} -p #{port} remove-device #{simnet}`
#   puts "src/vxadm -h #{host} -p #{port} remove-device #{simnet}" if verbose
#   puts output if verbose
# end