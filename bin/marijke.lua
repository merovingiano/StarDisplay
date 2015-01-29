print("  sourcing marijke.lua")

local marijke = {}
local counter = 1

function marijke.TakeSnapshot ()
  local Nodes = {}
  local Attribs = {}
  
  for p in Simulation.Prey() do
    local rt = p:reactionInterval()
    local dt = rt - p:reactionTime()
    Attribs[p.id] = { dt, rt }
    
    local Node = {}
    for ni in Simulation.Neighbors(p) do
	    if ni.interacting then
	      Node[#Node + 1] = ni.id
        if #Node > 6 then
          break
        end
	    end
    end
    Nodes[p.id] = Node
	end

  do 
    local fname = Simulation.DataPath .. "n" .. Simulation.GetFlock():num_prey() .. "_" .. counter .. "_attrib.cvs"
    local out = assert(io.open(fname, "w"))
    for k, t in ipairs(Attribs) do
      out:write(k, ',')
      out:write(table.concat(t, ','))
      out:write('\n')
    end
    out:close()
    print("Attributes saved in " .. fname)
  end  
  
  do 
    local fname = Simulation.DataPath .. "n" .. Simulation.GetFlock():num_prey() .. "_" .. counter .. ".cvs"
    local out = assert(io.open(fname, "w"))
    for k, t in ipairs(Nodes) do
      table.sort(t)
      for _, i in ipairs(t) do
        out:write(k, ',', i, '\n')
      end
    end
    out:close()
    print("Nodes saved in " .. fname)
  end  
  
  counter = counter + 1
end


return marijke
