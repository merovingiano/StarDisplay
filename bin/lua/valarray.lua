-- valarray.lua
--
-- A simple valarray implementation
--

print("  sourcing valarray.lua")


-- zip iterator
-- Mixed types (Lua array, valarray and numbers) are supported
--
function zip (A, B)
  local i = 0
  local n = 1
  if type(A) == "table" then
    n = #A
    if type(B) == "table" then
      return function ()
        i = i + 1
        if i <= n then return i, A[i], B[i] end
      end
    else
      return function ()
        i = i + 1
        if i <= n then return i, A[i], B end
      end
    end
  else   
    -- type(A) ~= "table"
    if type(B) == "table" then
      n = #B
      return function ()
        i = i + 1
        if i <= n then return i, A, B[i] end
      end
    else
      return function ()
        i = i + 1
        if i <= n then return i, A, B end
      end
    end
  end
end


local mt
mt = {
  __unm = function (A)
    local R = {}
    if type(A) == "number" then 
      A = {A}
    end
    for i,a in ipairs(A) do R[i] = -a end
    return setmetatable(R, mt)
  end,
  __add = function (A, B)
    local R = {}
    for i,a,b in zip(A,B) do R[i] = a + b end
    return setmetatable(R, mt)
  end,
  __sub = function (A, B)
    local R = {}
    for i,a,b in zip(A,B) do R[i] = a - b end
    return setmetatable(R, mt)
  end,
  __mul = function (A, B)
    local R = {}
    for i,a,b in zip(A,B) do R[i] = a * b end
    return setmetatable(R, mt)
  end,
  __div = function (A, B)
    local R = {}
    for i,a,b in zip(A,B) do R[i] = a / b end
    return setmetatable(R, mt)
  end,
  __pow = function (A, B)
    local R = {}
    for i,a,b in zip(A,B) do R[i] = a ^ b end
    return setmetatable(R, mt)
  end,
}


math.sign = function (x)
  return x>0 and 1 or x<0 and -1 or 0
end


local valarray = {}


-- va = valarray.new(100, 0)
-- creates an 100 element valarray with 100 zeros
--
-- va = valarray.new(A)
-- creates a deep copy of A. A could be an Lua array or an valarray.
--
function valarray.new (s, v)
  local R = {}
  if type(s) == "table" then
    for i,x in ipairs(s) do R[i] = x end
  else
    for i=1,s do R[i] = v end
  end
  return setmetatable(R, mt)
end


function valarray.linspace(lo, hi, n)
  local R = {}
  local inc = (hi - lo) / (n-1)
  for i = 1,n do
    R[i] = lo + (i-1) * inc
  end
  return setmetatable(R, mt)
end


function valarray.abs (A)
  local R = {}
  for i,x in ipairs(A) do R[i] = math.abs(x) end
  return setmetatable(R, mt)
end


function valarray.sqr (A)
  local R = {}
  for i,x in ipairs(A) do R[i] = x * x end
  return setmetatable(R, mt)
end


function valarray.log (A)
  local R = {}
  for i,x in ipairs(A) do R[i] = math.log(x) end
  return setmetatable(R, mt)
end


function valarray.log10 (A)
  local R = {}
  for i,x in ipairs(A) do R[i] = math.log10(x) end
  return setmetatable(R, mt)
end


function valarray.sign (A)
  local R = {}
  for i,x in ipairs(A) do R[i] = x>0 and 1 or x<0 and -1 or 0 end
  return setmetatable(R, mt)
end


-- Diff. The last element is duplicated
function valarray.diff (A)
  local Y = {}
  for i = 1, #A-1 do
    Y[i] = A[i+1] - A[i]
  end
  Y[#Y+1] = Y[#Y]
  return setmetatable(Y, mt)
end


function valarray.sum (A)
  local sum = 0
  for _, x in ipairs(A) do
    sum = sum + x
  end
  return sum
end


function valarray.minMaxMean (A)
  local min, max, sum, n = math.huge, -math.huge, 0, 0
  for _, x in ipairs(A) do
    n = n + 1
    sum = sum + x
    min = math.min(min, x)
    max = math.max(max, x)
  end
  return min, max, sum / n
end


-- Standard deviation
function valarray.sd (A)
  local _, _, mean = valarray.minMaxMean(A)
  local var = 0
  for _, x in ipairs(A) do
    var = var + (x - mean) * (x - mean)
  end
  return math.sqrt(var / #A)
end


-- Coefficient of variation
function valarray.CV (A)
  local _, _, mean = valarray.minMaxMean(A)
  local var = 0
  for _, x in ipairs(A) do
    var = var + (x - mean) * (x - mean)
  end
  return math.sqrt(var / (#A-1)) / mean
end


-- Quantile
function valarray.quantile (A, q)
  local Y = {}
  for i,x in ipairs(A) do Y[i] = x end
  table.sort(Y)
  return Y[math.floor(q * #Y)]
end


-- Returns an valarray consisting of '1' for each element in A that
-- is greater or equal to threshol, '0' otherwise.
function valarray.boolean (A, threshold)
  local Y = {} 
  for i, x in ipairs(A) do
    Y[i] = (x >= threshold) and 1 or 0
  end
  return setmetatable(Y, mt)
end


-- res_i <- clamp(A_i, lo, hi) / (hi - lo)
-- 
-- Default values for lo and hi are:
-- lo, hi = min(A), max(A)
function valarray.normalize (A, hi, lo)
  local Y = {}
  local min, max = valarray.minMaxMean(A)
  max = hi or max
  min = lo or min
  local scale = 1 / (max - min)
  for i, x in ipairs(A) do
    x = (x > max) and max or x
    x = (x < min) and min or x
    Y[i] = (x - min) * scale
  end
  return setmetatable(Y, mt)
end


function valarray.supersample (A, I)
  local R = {}
  for i = 1,#A-1 do
    local xa, xb = I[i], I[i+1]
    local ya, yb = A[i], A[i+1]
    local x = xa
    while x < xb do
      local y = ya + (yb - ya) * (x - xa) / (xb - xa)
      R[#R+1] = y
      x = x + 1
    end
  end
  R[#R+1] = A[#A]
  return setmetatable(R, mt)
end


function valarray.transpose (A)
  if type(A[1]) ~= "table" then return valarray.new(A) end
  local R = {}
  for i = 1,#A[1] do
    R[i] = setmetatable({}, mt)
    for j = 1,#A do
      R[i][j] = A[j][i]
    end
  end
  return setmetatable(R, mt)
end


-- Y = A + BX
-- returns A,B
--
function valarray.linear_regression(X, Y)
  local _,_,xm = valarray.minMaxMean(X)
  local _,_,ym = valarray.minMaxMean(Y)
  local rX = X - xm
  local rY = Y - ym
  local b = valarray.sum( rX * rY ) / valarray.sum( rX * rX )
  local a = ym - b * xm
  return a, b
end





return valarray



