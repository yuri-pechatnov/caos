    
double my_exp(double x) { 
    double xn = 1.0, fac = 1.0, part = 1.0, result = 1.0, old_result = 0.0;
    for (int i = 2; result != old_result; ++i) {
        old_result = result;
        result += part;
        fac *= i;
        xn *= x;
        part = xn / fac;
    }
    return result;
}

