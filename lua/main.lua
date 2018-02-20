local get_table = require 'get_table'

testtable = {1,3,5,7, ["a"] = 11, bcd = 15, ac = "a"}
testtable.a = 12

result = get_table(testtable)

io.write(#result, "\n")
for _,v in ipairs(result) do
    io.write(v[1]," ", v[2], "\n")
end
