#!/bin/bash

# Script to fix Makefiles that use g++ instead of icpx
# Adds explicit %.o: %.cpp rule to use $(CC)

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0
SKIPPED=0
ALREADY_OK=0

echo "========================================="
echo "Fixing Makefile Compiler Issues"
echo "========================================="
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    makefile="$bench/Makefile"

    if [ ! -f "$makefile" ]; then
        echo "⚠️  $bench: Makefile not found"
        SKIPPED=$((SKIPPED + 1))
        continue
    fi

    # Check if explicit %.o: %.cpp rule already exists
    if grep -q "^%.o.*:.*%.cpp" "$makefile"; then
        echo "✓  $bench: Already has explicit rule"
        ALREADY_OK=$((ALREADY_OK + 1))
        continue
    fi

    # Backup the Makefile
    cp "$makefile" "$makefile.bak"

    # Add explicit compilation rule before the clean target
    # This ensures $(CC) is used instead of g++
    if grep -q "^clean:" "$makefile"; then
        sed -i '/^clean:/i\
%.o: %.cpp\
\t$(CC) $(CFLAGS) -c $< -o $@\
' "$makefile"
        echo "✅ $bench: Fixed"
        FIXED=$((FIXED + 1))
    else
        # If no clean target, add at the end before any run/test targets
        echo "" >> "$makefile"
        echo "# Explicit compilation rule to use \$(CC) instead of g++" >> "$makefile"
        echo "%.o: %.cpp" >> "$makefile"
        echo -e "\t\$(CC) \$(CFLAGS) -c \$< -o \$@" >> "$makefile"
        echo "" >> "$makefile"
        echo "✅ $bench: Fixed (added at end)"
        FIXED=$((FIXED + 1))
    fi
done

echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "Fixed:         $FIXED"
echo "Already OK:    $ALREADY_OK"
echo "Skipped:       $SKIPPED"
echo "Total:         $(wc -l < errors_wrong_compiler.txt)"
echo ""

if [ $FIXED -gt 0 ]; then
    echo "✅ Makefiles have been fixed!"
    echo ""
    echo "Next step: Recompile the fixed benchmarks"
    echo ""
    echo "To recompile all fixed benchmarks:"
    echo "  ./recompile_fixed.sh"
    echo ""
    echo "Backups saved as Makefile.bak in each directory"
fi
