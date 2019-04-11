gl_version = {abc = "def", [0] = 0, [1] = 1.5}

-- io = require('io')


function call_cpp()
    lambda_test()
    lambda_test2(21, false)
    lambda_test3(23.4)
    function_test(123)
    io.write(cpp.var_test .. "\n")
    
    obj_test = new_object(2, true)
    obj_test:printi()
end
