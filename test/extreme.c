int main() {
    // Example arrays
    int int_array[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    double double_array[] = {3.1, 4.1, 5.9, 2.6, 5.3, 5.0};

    size_t int_size = sizeof(int_array) / sizeof(int_array[0]);
    size_t double_size = sizeof(double_array) / sizeof(double_array[0]);

    // Find min and max for int array
    int* int_min = (int*)find_extreme(int_array, sizeof(int), int_size, compare_int_min);
    int* int_max = (int*)find_extreme(int_array, sizeof(int), int_size, compare_int_max);

    // Find min and max for double array
    double* double_min = (double*)find_extreme(double_array, sizeof(double), double_size, compare_double_min);
    double* double_max = (double*)find_extreme(double_array, sizeof(double), double_size, compare_double_max);

    // Print results
    printf("Int array: Min = %d, Max = %d\n", *int_min, *int_max);
    printf("Double array: Min = %.1f, Max = %.1f\n", *double_min, *double_max);

    return 0;
}
