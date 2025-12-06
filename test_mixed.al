func test_mixed(): float {
    int x = 10;
    float y = 20.5;
    float z = x + y;
    print(z);
    return z;
}

func main(): void {
    test_mixed();
}
