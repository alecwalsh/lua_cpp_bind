return function(t)
    local out = {}
    local outidx = 1
    local count = 0
    for k,v in pairs(t) do
        out[outidx] = {k,v}
        outidx = outidx + 1
        count = count + 1
    end
    return setmetatable(out, {__len = function() return count end})
end
