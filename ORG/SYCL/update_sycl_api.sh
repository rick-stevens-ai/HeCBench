#!/bin/bash

# Script to update deprecated SYCL 1.2.1 API to SYCL 2020 API
# Updates accessor syntax and access modes

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0
SKIPPED=0
NO_CHANGES=0

echo "========================================="
echo "Updating SYCL API from 1.2.1 to 2020"
echo "========================================="
echo ""

for bench in $(cat errors_deprecated_api.txt); do
    if [ ! -d "$bench" ]; then
        echo "⚠️  $bench: Directory not found"
        SKIPPED=$((SKIPPED + 1))
        continue
    fi

    echo "Processing: $bench"

    # Find all .cpp, .h, .hpp files
    files=$(find "$bench" -name "*.cpp" -o -name "*.h" -o -name "*.hpp")

    if [ -z "$files" ]; then
        echo "  ⚠️  No source files found"
        SKIPPED=$((SKIPPED + 1))
        continue
    fi

    changed=0

    for file in $files; do
        # Backup the file
        cp "$file" "$file.bak_sycl2020"

        # Check if file has deprecated API
        if ! grep -q "sycl_read\|sycl_write\|access::target::local" "$file"; then
            rm "$file.bak_sycl2020"
            continue
        fi

        # Apply fixes using sed

        # 1. Fix local accessor declarations
        # accessor<TYPE, DIM, sycl_read_write, access::target::local> NAME(SIZE, cgh);
        # -> local_accessor<TYPE, DIM> NAME(SIZE, cgh);
        sed -i 's/accessor<\([^,]*\), \([0-9]*\), sycl_read_write, access::target::local>/local_accessor<\1, \2>/g' "$file"
        sed -i 's/accessor<\([^,]*\), \([0-9]*\), sycl_write, access::target::local>/local_accessor<\1, \2>/g' "$file"
        sed -i 's/accessor<\([^,]*\), \([0-9]*\), sycl_read, access::target::local>/local_accessor<\1, \2>/g' "$file"

        # 2. Fix get_access calls
        # get_access<sycl_read> -> get_access<access_mode::read>
        sed -i 's/get_access<sycl_read>/get_access<access_mode::read>/g' "$file"
        sed -i 's/get_access<sycl_write>/get_access<access_mode::write>/g' "$file"
        sed -i 's/get_access<sycl_read_write>/get_access<access_mode::read_write>/g' "$file"
        sed -i 's/get_access<sycl_discard_write>/get_access<access_mode::discard_write>/g' "$file"
        sed -i 's/get_access<sycl_discard_read_write>/get_access<access_mode::discard_read_write>/g' "$file"

        # 3. Add using declaration if needed (at the top after includes)
        if grep -q "local_accessor\|access_mode::" "$file" && ! grep -q "using namespace sycl" "$file"; then
            # Check if it's a header or source file
            if [[ "$file" == *.cpp ]]; then
                # For .cpp files, add after last #include
                sed -i '/^#include/a\
using namespace sycl;' "$file"
            fi
        fi

        # Check if file was actually changed
        if ! diff -q "$file" "$file.bak_sycl2020" > /dev/null 2>&1; then
            echo "  ✅ Updated: $(basename $file)"
            changed=1
        else
            rm "$file.bak_sycl2020"
        fi
    done

    if [ $changed -eq 1 ]; then
        FIXED=$((FIXED + 1))
        echo "  ✓ $bench: Updated"
    else
        NO_CHANGES=$((NO_CHANGES + 1))
        echo "  ℹ️  $bench: No changes needed"
    fi

    echo ""
done

echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "Benchmarks updated:      $FIXED"
echo "No changes needed:       $NO_CHANGES"
echo "Skipped:                 $SKIPPED"
echo "Total processed:         $(wc -l < errors_deprecated_api.txt)"
echo ""

if [ $FIXED -gt 0 ]; then
    echo "✅ SYCL API updated to 2020!"
    echo ""
    echo "Backups saved as *.bak_sycl2020 in each directory"
    echo ""
    echo "Next step: Recompile the updated benchmarks"
    echo "  ./recompile_sycl2020.sh"
fi
