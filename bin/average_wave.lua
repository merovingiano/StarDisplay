print("average_wave.lua\n")


local DataPath = Simulation.DataPath
local out = io.open(DataPath .. "average_wave.dat", "w")    -- Output file


--
-- Define accumulator factories
--
local ammm = Analysis.min_max_mean_N
local ahist = function () return Analysis.histogram(0,100,10000) end


function Accumulate(S)
  local n = #S
  local mmm = { ammm(), ammm(), ammm(), ammm(), ammm(), ammm(), ammm(), ammm() }
  local hist = { ahist(), ahist(), ahist(), ahist(), ahist(), ahist(), ahist(), ahist() }
  for i = math.ceil(n/2), n do
    for j, v in ipairs(S[i]) do
	    mmm[j](v)
      hist[j](v)
	  end
  end
  return mmm, hist
end


function ProcessDataFile (fn)
  io.write("reading ", fn, "\n")

  -- Extract run and repetition from filename
  local _,_,run,rep = string.find(fn, "wave_(%d+)_(%d+)")

  -- Read data file line by line into table S
  local S = {}
  for line in io.lines(DataPath .. fn) do
    local ind = {}
    local _,_,dt,nndT0,nndT1,D0,D1,pc,V0,V1 = string.find(line, "(.+) (.+) (.+) (.+) (.+) (.+) (.+) (.+)")
	  ind = {dt,nndT0,nndT1,D0,D1,pc,V0,V1}
	  for i, v in ipairs(ind) do ind[i] = tonumber(v) end
	  S[#S + 1] = ind
  end

  -- Do the hard stuff
  local mmm, hist = Accumulate(S)
  
  -- A line in the output file consist of:
  --
  -- run repetition samples duration (for each in (nndT0, nnT1, D0, D1, V0, V1):) min mean max q25 q50 q75
  --
  out:write(run, ' ', rep, ' ', mmm[1]:count(), ' ', mmm[1]:max(), ' ')
  for i=2,8 do
    out:write(mmm[i]:min(), ' ', mmm[i]:mean(), ' ', mmm[i]:max(), ' ')
    local q = hist[i]:quartiles()
  	out:write(q.x, ' ', q.y, ' ', q.z, ' ')
  end
  out:write('\n') 
end


--
-- Main driver
--
os.execute("dir " .. DataPath .. "wave*.dat /B >average_wave_files.tmp")
for fn in io.lines("average_wave_files.tmp") do
  ProcessDataFile(fn)
end
os.remove("average_wave_files.tmp")
out:close()
io.write("\nDone!\n\n")

