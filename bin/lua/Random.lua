print("  sourcing Random.lua")
-- random.lua
-- 
-- Random number generator & random distributions

--[[


Seeds the generator with s
function random:seed(s)  

Returns the seed value
function random:getSeed ()

Returns uniformly distributed random value in [0, 1)
function random:uniform01()

Returns uniformly distributed random value in [min, max)
function random:uniform (min, max)

Returns normal distributed random value
function random:normal (mean, sigma)

Returns geometric distributed random value 
function random:geometric (P)

Returns random unit vector
function random:unit_vec()

Returns random vector in unit sphere
function vec_in_sphere()

--]]


function random:normal_min_max (mean, sigma, min, max)
  local r = self:normal(mean, sigma)
  while r < min or r >= max do
    r = self:normal(mean, sigma)
  end
  return r
end


return random
