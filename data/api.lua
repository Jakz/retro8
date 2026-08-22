function all(t)
  if t == nil then
    return function() end
  end

  local i = 0
  local last_i = 0
  local last = nil

  return function()
    if last ~= nil and t[last_i] ~= last then
      i = last_i - 1
    end

    i = i + 1
    last_i = i
    last = t[i]
    return last
  end
end

function add(t, v, i)
  if t ~= nil then
    if i == nil then
      t[#t+1] = v
    else
      table.insert(t, i, v)
    end
    return v
  end
end

function foreach(c, f)
  if c ~= nil then
    for value in all(c) do
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
  return 0
end

function del(t, v)
  if t ~= nil then
    for i = 1, #t do
      if t[i] == v then
        table.remove(t, i)
        return v
      end
    end
  end
end

function deli(t, i)
  if t ~= nil then
    i = i or #t
    if i >= 1 and i <= #t then
      return table.remove(t, i)
    end
  end
end

function cocreate(f)
  return coroutine.create(f)
end

function yield(...)
  return coroutine.yield(...)
end

function coresume(f, ...)
  return coroutine.resume(f, ...)
end

function costatus(f)
  return coroutine.status(f)
end
