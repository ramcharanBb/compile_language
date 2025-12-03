func test_dce_targets(): void {

    int unused_var = 10 * 5;

    print("Starting DCE tests...");

    if (1 == 2) { 
        
        print("You should never see this message."); 
        int unreachable_data = 12345;               
        print(unreachable_data);
    } else {
        
        print("The 'else' branch is the only reachable path.");
    }


    print("About to return...");
    return;
    
    int this_is_dead = 999; 
    this_is_dead = this_is_dead + 1;
    print("This code is unreachable and should be eliminated!");
    if (true) {
        print("More dead code.");
    }
}


func main(): void {
    test_dce_targets();
    print("Done!");
}
