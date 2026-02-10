#!/bin/bash
# Convenience script for running variational tests

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TEST_DIR="$SCRIPT_DIR/../.."

cd "$TEST_DIR" || exit 1

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}→ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Parse command line arguments
CATEGORY=""
VERBOSE=""
PARALLEL="-j 4"

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--category)
            CATEGORY="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE="--verbose"
            shift
            ;;
        -s|--serial)
            PARALLEL=""
            shift
            ;;
        -q|--quick)
            CATEGORY="quick"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -c, --category CAT   Run specific category (diffusion, allen_cahn, etc.)"
            echo "  -q, --quick          Run only quick smoke tests"
            echo "  -v, --verbose        Verbose output"
            echo "  -s, --serial         Run tests serially (no parallel)"
            echo "  -h, --help           Show this help"
            echo ""
            echo "Categories:"
            echo "  quick                Quick smoke tests (< 10s)"
            echo "  diffusion            Diffusion equation tests"
            echo "  allen_cahn           Allen-Cahn phase field tests"
            echo "  cahn_hilliard        Cahn-Hilliard tests"
            echo "  splitting            Variable splitting tests"
            echo "  coupled              Coupled multi-physics tests"
            echo "  gray_scott           Gray-Scott reaction-diffusion"
            echo "  var_diff             var_diff() operator tests"
            echo "  all                  All tests (default)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h for help"
            exit 1
            ;;
    esac
done

# Build test executable if needed
if [ ! -f "moose_test-opt" ]; then
    print_info "Building test executable..."
    make -j4 || {
        print_error "Build failed!"
        exit 1
    }
    print_success "Build complete"
fi

# Run tests based on category
case "$CATEGORY" in
    quick)
        print_header "Running Quick Smoke Tests"
        ./run_tests -i tests/variational -re "quick|test_verbose" $PARALLEL $VERBOSE
        ;;
    diffusion)
        print_header "Running Diffusion Tests"
        ./run_tests -i tests/variational -re "diffusion|poisson" $PARALLEL $VERBOSE
        ;;
    allen_cahn)
        print_header "Running Allen-Cahn Tests"
        ./run_tests -i tests/variational -re "allen_cahn" $PARALLEL $VERBOSE
        ;;
    cahn_hilliard)
        print_header "Running Cahn-Hilliard Tests"
        ./run_tests -i tests/variational -re "cahn_hilliard" $PARALLEL $VERBOSE
        ;;
    splitting)
        print_header "Running Variable Splitting Tests"
        ./run_tests -i tests/variational -re "split" $PARALLEL $VERBOSE
        ;;
    coupled)
        print_header "Running Coupled Tests"
        ./run_tests -i tests/variational -re "coupled" $PARALLEL $VERBOSE
        ;;
    gray_scott)
        print_header "Running Gray-Scott Tests"
        ./run_tests -i tests/variational -re "gray_scott" $PARALLEL $VERBOSE
        ;;
    var_diff)
        print_header "Running var_diff() Tests"
        ./run_tests -i tests/variational -re "var_diff|double_well|polynomial" $PARALLEL $VERBOSE
        ;;
    all|"")
        print_header "Running All Variational Tests"
        ./run_tests -i tests/variational $PARALLEL $VERBOSE
        ;;
    *)
        print_error "Unknown category: $CATEGORY"
        echo "Use -h for list of categories"
        exit 1
        ;;
esac

EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    print_success "All tests passed!"
else
    print_error "Some tests failed (exit code: $EXIT_CODE)"
fi

exit $EXIT_CODE
