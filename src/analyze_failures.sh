#!/bin/bash

LOGDIR="compilation_logs_20260108_102500"

echo "Analyzing failure patterns..."
echo ""

# Initialize category files
> category_deprecated_api.txt
> category_pointer_issues.txt
> category_namespace_issues.txt
> category_external_deps.txt
> category_template_issues.txt
> category_other.txt

# Analyze each failed compilation
while read bench; do
    log="$LOGDIR/${bench}.log"
    
    if [ ! -f "$log" ]; then
        continue
    fi
    
    # Check for deprecated API
    if grep -q "deprecated.*local_accessor\|access::target::local" "$log" 2>/dev/null; then
        echo "$bench" >> category_deprecated_api.txt
    # Check for pointer issues
    elif grep -q "no known conversion.*ptr\|get_pointer" "$log" 2>/dev/null; then
        echo "$bench" >> category_pointer_issues.txt
    # Check for namespace issues
    elif grep -q "no member named.*in namespace\|std::fabs\|sycl::fabs" "$log" 2>/dev/null; then
        echo "$bench" >> category_namespace_issues.txt
    # Check for external dependencies
    elif grep -qi "CImg\|opencv\|png.h\|jpeg" "$log" 2>/dev/null; then
        echo "$bench" >> category_external_deps.txt
    # Check for template issues
    elif grep -q "template.*error\|no matching function\|candidate" "$log" 2>/dev/null; then
        echo "$bench" >> category_template_issues.txt
    else
        echo "$bench" >> category_other.txt
    fi
done < failed_compile.txt

# Print summary
echo "Error Categories:"
echo "================="
echo "Deprecated API:       $(wc -l < category_deprecated_api.txt 2>/dev/null || echo 0)"
echo "Pointer issues:       $(wc -l < category_pointer_issues.txt 2>/dev/null || echo 0)"
echo "Namespace issues:     $(wc -l < category_namespace_issues.txt 2>/dev/null || echo 0)"
echo "External deps:        $(wc -l < category_external_deps.txt 2>/dev/null || echo 0)"
echo "Template issues:      $(wc -l < category_template_issues.txt 2>/dev/null || echo 0)"
echo "Other/Complex:        $(wc -l < category_other.txt 2>/dev/null || echo 0)"
echo "No binary (special):  $(wc -l < failed_no_binary.txt)"
echo ""
