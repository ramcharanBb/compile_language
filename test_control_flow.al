func test_no_dce(): void {
    int x = 10;
    int y = 20;
    
    int z = x + y; 
    print(z);

    if (x < y) {
        print("x is less than y");
    } else {
        print("x is greater or equal");
    }
    
    int limit = z * 2; 
    
    int count = 0;
    while (count < 5) {
        print(count);
        if (count == 4) { print(limit); } 
        count = count + 1;
    }
    
    if (true) {
        print("This always runs A");
    } else {
        print("Unreachable print A, but has side effect.");
    }
    
    if (false) {
        print("Unreachable print B, but has side effect.");
    } else {
        print("This always runs B");
    }
    
    
    int magic_num = (5 + 7) * 3; 
    print(magic_num);

    return;
}

func main(): void {
    test_no_dce();
    print("Done!");
}
