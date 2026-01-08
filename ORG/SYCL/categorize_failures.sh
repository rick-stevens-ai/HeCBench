#!/bin/bash

echo "Categorizing Failure Types"
echo "=========================="
echo

cd /home/stevens/HeCBench/ORG/SYCL

# Create category files
> sycl_api_issues.txt
> missing_headers.txt
> undeclared_identifiers.txt
> namespace_issues.txt
> recursive_kernel.txt
> syntax_errors.txt
> other_errors.txt

# Categorize each failure
while read bench; do
  log="compilation_logs/${bench}_test.log"
  if [ -f "$log" ]; then
    if grep -q "no member named.*shuffle" "$log"; then
      echo "$bench (shuffle_down API)" >> sycl_api_issues.txt
    elif grep -q "no member named.*ctz" "$log"; then
      echo "$bench (ctz extension)" >> sycl_api_issues.txt
    elif grep -q "file not found\|No such file" "$log"; then
      missing_file=$(grep -oP "'\K[^']+(?='.*file not found)" "$log" | head -1)
      echo "$bench (missing: $missing_file)" >> missing_headers.txt
    elif grep -q "use of undeclared identifier" "$log"; then
      undeclared=$(grep -oP "use of undeclared identifier '\K[^']+" "$log" | head -1)
      echo "$bench (undeclared: $undeclared)" >> undeclared_identifiers.txt
    elif grep -q "recursive function" "$log"; then
      echo "$bench" >> recursive_kernel.txt
    elif grep -q "unknown type name.*float[234]" "$log"; then
      echo "$bench (CUDA float types)" >> syntax_errors.txt
    elif grep -q "expected ')'\|expected ';'" "$log"; then
      echo "$bench (syntax)" >> syntax_errors.txt
    elif grep -q "ambiguous\|no matching" "$log"; then
      echo "$bench (overload/namespace)" >> namespace_issues.txt
    else
      error=$(grep -E "error:" "$log" | head -1 | cut -c1-100)
      echo "$bench | $error" >> other_errors.txt
    fi
  fi
done < still_failing.txt

# Also process the first 20 that failed
for bench in bsw btree complex convolutionDeformable daphne depixel diamond dwconv1d dwt2d eikonal frechet halo-finder hbc; do
  bench="${bench}-sycl"
  log="compilation_logs/${bench}_test.log"
  if [ -f "$log" ]; then
    if grep -q "no member named.*shuffle" "$log"; then
      echo "$bench (shuffle_down API)" >> sycl_api_issues.txt
    elif grep -q "no member named.*ctz" "$log"; then
      echo "$bench (ctz extension)" >> sycl_api_issues.txt
    elif grep -q "file not found\|No such file" "$log"; then
      missing_file=$(grep -oP "'\K[^']+(?='.*file not found)" "$log" | head -1)
      echo "$bench (missing: $missing_file)" >> missing_headers.txt
    elif grep -q "use of undeclared identifier" "$log"; then
      undeclared=$(grep -oP "use of undeclared identifier '\K[^']+" "$log" | head -1)
      echo "$bench (undeclared: $undeclared)" >> undeclared_identifiers.txt
    elif grep -q "recursive function" "$log"; then
      echo "$bench" >> recursive_kernel.txt
    elif grep -q "unknown type name.*float[234]" "$log"; then
      echo "$bench (CUDA float types)" >> syntax_errors.txt
    elif grep -q "expected ')'\|expected ';'" "$log"; then
      echo "$bench (syntax)" >> syntax_errors.txt
    elif grep -q "ambiguous\|no matching" "$log"; then
      echo "$bench (overload/namespace)" >> namespace_issues.txt
    else
      error=$(grep -E "error:" "$log" | head -1 | cut -c1-80)
      echo "$bench | $error" >> other_errors.txt
    fi
  fi
done

echo "Category Summary:"
echo "================="
echo
echo "1. SYCL API Issues ($(wc -l < sycl_api_issues.txt)):"
cat sycl_api_issues.txt
echo
echo "2. Missing Headers/Files ($(wc -l < missing_headers.txt)):"
cat missing_headers.txt
echo
echo "3. Undeclared Identifiers ($(wc -l < undeclared_identifiers.txt)):"
cat undeclared_identifiers.txt
echo
echo "4. Recursive Kernel Issues ($(wc -l < recursive_kernel.txt)):"
cat recursive_kernel.txt
echo
echo "5. Syntax/Type Errors ($(wc -l < syntax_errors.txt)):"
cat syntax_errors.txt
echo
echo "6. Namespace/Overload Issues ($(wc -l < namespace_issues.txt)):"
cat namespace_issues.txt
echo
echo "7. Other Errors ($(wc -l < other_errors.txt)):"
cat other_errors.txt
