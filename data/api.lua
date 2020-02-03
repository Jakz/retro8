function all(t)
  if t ~= nil then
    local nt = {}
    local ni = 1
    for _,v in pairs(t) do
      nt[ni] = v
      ni = ni + 1
    end
    for k,v in pairs(nt) do
    end

    local i = 0
    return function() i = i + 1; return nt[i] end
  end
  return function() end
end

function add(t, v)
  if t ~= nil then
    t[#t+1] = v
    return v
  end
end

function foreach(c, f)
  if c ~= nil then
    for key, value in ipairs(c) do
      f(value)
    end
  end
end


function mapdraw(...)
  map(table.unpack(arg))
end

function count(t)
  if t ~= nil then
    return #t
  end
end

function del(t, v)
  if t ~= nil then
    local found = false
    for i = 1, #t do
      if t[i] == v then
        found = true
      end
      if found then
        t[i] = t[i+1]
      end
    end
    if found then
      return v
    end
  end
end

function cocreate(f)
  return coroutine.create(f)
end

function yield()
  coroutine.yield()
end

-- TODO: missing vararg
function coresume(f)
  return coroutine.resume(f)
end

function costatus(f)
  return coroutine.status(f)
end