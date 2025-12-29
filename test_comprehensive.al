func test_arithmetic(): void {
    print("Testing Arithmetic:");
    int a = 10;
    int b = 3;
    int sum = a + b;
    int diff = a - b;
    int prod = a * b;
    int quot = a / b;
    int rem = a % b;
    print("10 + 3 ="); print(sum);
    print("10 - 3 ="); print(diff);
    print("10 * 3 ="); print(prod);
    print("10 / 3 ="); print(quot);
    print("10 % 3 ="); print(rem);

    float x = 10.5;
    float y = 2.0;
    float fsum = x + y;
    float fdiff = x - y;
    float fprod = x * y;
    float fquot = x / y;
    print("10.5 + 2.0 ="); print(fsum);
    print("10.5 - 2.0 ="); print(fdiff);
    print("10.5 * 2.0 ="); print(fprod);
    print("10.5 / 2.0 ="); print(fquot);
}

func test_control_flow(): void {
    print("Testing Control Flow:");
    int x = 5;
    if (x < 10) {
        print("5 is less than 10 (Correct)");
    } else {
        print("5 is not less than 10 (Incorrect)");
    }

    if (x > 10) {
        print("5 is greater than 10 (Incorrect)");
    } else {
        print("5 is not greater than 10 (Correct)");
    }

    int count = 0;
    print("Counting to 3:");
    while (count < 3) {
        print(count);
        count = count + 1;
    }
}

func add_ints(a: int, b: int): int {
    return a + b;
}

func add_floats(a: float, b: float): float {
    return a + b;
}

func test_functions(): void {
    print("Testing Functions:");
    int res_int = add_ints(5, 7);
    print("add_ints(5, 7) ="); print(res_int);

    float res_float = add_floats(5.5, 2.5);
    print("add_floats(5.5, 2.5) ="); print(res_float);
}

func main(): int {
    print("--- START COMPREHENSIVE TEST ---");
    test_arithmetic();
    test_control_flow();
    test_functions();
    print("--- END COMPREHENSIVE TEST ---");
    return 0;
}
