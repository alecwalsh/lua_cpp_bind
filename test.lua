-- gl_version = {3, 3, false, abc = "def"}
-- 
-- table2 = {[1]=1, [2]=3, [gl_version] = 4}
-- io = require('io')

function call_cpp()
    test(21, false)
    io.write(test.numargs, "\n")
end
