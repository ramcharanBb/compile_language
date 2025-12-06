func test_float(): float {
    float x = 10.5;
    float y = 2.0;
    x = x * y + 1.5;
    print(x);
    return x;
}

func main(): void {
    test_float();
}
