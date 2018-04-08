gl_version = {3, 3, false, abc = "def"}
-- 
-- table2 = {[1]=1, [2]=3, [gl_version] = 4}
-- io = require('io')

function call_cpp()
    lambda_test()
    lambda_test2(21, false)
--     lambda_test3(21.5)
    function_test(123)
    io.write(cpp.var_test)
end
