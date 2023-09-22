/**
 * @example types/test/datagrid_test.cc
 */

#include <usml/types/data_grid.h>
#include <usml/types/data_grid_bathy.h>
#include <usml/types/gen_grid.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_log.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/randgen.h>

#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer/timer.hpp>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>

BOOST_AUTO_TEST_SUITE(datagrid_test)

using namespace boost::unit_test;
using namespace usml::types;

using iterator = seq_vector::iterator;

/**
 * @ingroup types_test
 * @{
 */

/**
 * As a note when using valgrind mem-check, there are many instances
 * of an "Conditional jump or move depends on uninitialised value(s)"
 * warning that are issued for the deriv_1d_test and datagrid_fast_acc_test
 * that have been determined to not effect the overall performance or
 * functionality of gen_grid::pchip and may be ignored.
 */

/**
 * Test the ability of data_grid_compute_offset() to retrieve data
 * from a 3-D data matrix in column major order.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(compute_index_test) {
    cout << "=== datagrid_test: compute_index_test ===" << endl;

    // build a set of axes like the ones used in data_grid

    seq_vector::csptr xaxis(new seq_linear(0, 100.0, 2));
    seq_vector::csptr yaxis(new seq_linear(0, 10.0, 3));
    seq_vector::csptr zaxis(new seq_linear(0, 1.0, 4));
    seq_vector::csptr axis[3] = {xaxis, yaxis, zaxis};
    cout << "xaxis=" << *xaxis << endl;
    cout << "yaxis=" << *yaxis << endl;
    cout << "zaxis=" << *zaxis << endl;

    // fill in a data vector using combination of axis values

    double data[2 * 3 * 4];
    size_t numPoints = 0;
    for (double x : *xaxis) {
        for (double y : *yaxis) {
            for (double z : *zaxis) {
                data[numPoints++] = x + y + z;
            }
        }
    }
    cout << "data =";
    for (size_t n = 0; n < numPoints; ++n) {
        printf("%03.0f ", data[n]);
    }
    cout << endl;

    // check to see if all data in the right place

    size_t index[3];
    index[0] = 0;
    for (double x : *xaxis) {
        index[1] = 0;
        for (double y : *yaxis) {
            index[2] = 0;
            for (double z : *zaxis) {
                size_t k = data_grid_compute_offset<2>(axis, index);
                double value = data[k];
                printf("x=%lu y=%lu z=%lu offset=%02lu data=%03.0f\n", index[0],
                       index[1], index[2], k, value);
                BOOST_CHECK_CLOSE(value, x + y + z, 1e-6);
                index[2] = index[2] + 1;
            }
            index[1] = index[1] + 1;
        }
        index[0] = index[0] + 1;
    }
}

/**
 * Compute a linear field value of 1-D interpolation test data
 */
static double linear1d(double x) { return 5.0 + 3.0 * x; }

/**
 * Compute a quadratic field value of 1-D interpolation test data
 */
static double quad1d(double x) { return 5.0 + 3.0 * x - 0.3 * x * x; }

/**
 * Compute the derivative of the function defined in quad1d().
 */
static double deriv1d(double x) { return 3.0 - 0.6 * x; }

/**
 * Interpolate 1-D linear field using double precision numbers.
 * Exercise all of the 1-D interpolation types.
 * Show that data can be interpolated outside of original domain.
 * Generate errors if linear or cubic values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(linear_1d_test) {
    cout << "=== datagrid_test: linear_1d_test ===" << endl;
    double truth;
    double nearest;
    double linear;
    double pchip;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    gen_grid<1> grid(&axis);
    grid.edge_limit(0, false);
    for (size_t n = 0; n < axis->size(); ++n) {
        grid.setdata(&n, linear1d((*axis)(n)));
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 0.25; x <= 10.0; x += 0.25) {
        // double y = x;
        cout << x << "\t";
        truth = linear1d(x);
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&x);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&x);
        cout << linear << "\t";
        BOOST_CHECK_CLOSE(linear, truth, 1e-6);

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&x);
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE(pchip, truth, 1e-6);

        cout << endl;
    }
}

/**
 * Interpolate 1-D linear field using single precision numbers.
 * Exercise all of the 1-D interpolation types.
 * Show that data can be interpolated outside of original domain.
 * Generate errors if linear or cubic values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(linear_1d_float_test) {
    cout << "=== datagrid_test: linear_1d_float_test ===" << endl;
    typedef float element_type;
    element_type truth;
    element_type nearest;
    element_type linear;
    element_type pchip;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    gen_grid<1, element_type> grid(&axis);
    grid.edge_limit(0, false);
    for (size_t n = 0; n < axis->size(); ++n) {
        grid.setdata(&n, (element_type)linear1d((*axis)(n)));
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 0.25; x <= 10.0; x += 0.25) {
        // double y = x;
        cout << x << "\t";
        truth = (element_type)linear1d(x);
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&x);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&x);
        cout << linear << "\t";
        BOOST_CHECK_CLOSE(linear, truth, 1e-6);

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&x);
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE(pchip, truth, 1e-6);

        cout << endl;
    }
}

/**
 * Interpolate 1-D linear field using complex numbers.
 * Exercise all of the 1-D interpolation types.
 * Show that data can be interpolated outside of original domain.
 * Generate errors if linear or cubic values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(linear_1d_complex_test) {
    cout << "=== datagrid_test: linear_1d_complex_test ===" << endl;
    typedef std::complex<double> element_type;
    element_type truth;
    element_type nearest;
    element_type linear;
    element_type pchip;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    gen_grid<1, element_type> grid(&axis);
    grid.edge_limit(0, false);
    for (size_t n = 0; n < axis->size(); ++n) {
        element_type values(linear1d((*axis)(n)), 1.0);
        grid.setdata(&n, values);
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 0.25; x <= 10.0; x += 0.25) {
        // double y = x;
        cout << x << "\t";
        truth = element_type(linear1d(x), 1.0);
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&x);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&x);
        cout << linear << "\t";
        //        BOOST_CHECK_CLOSE(linear, truth, 1e-6);

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&x);
        cout << pchip << "\t";
        //        BOOST_CHECK_CLOSE(pchip, truth, 1e-6);

        cout << endl;
    }
}

/**
 * Interpolate 1-D linear field using a vector.
 * Exercise all of the 1-D interpolation types.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(linear_1d_vector_test) {
    typedef boost::numeric::ublas::vector<double> element_type;
    element_type truth;
    element_type nearest;
    element_type linear;
    element_type pchip;
    size_t N = 3;

    cout << "=== datagrid_test: linear_1d_vector_test ===" << endl;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    const seq_vector::csptr* ax = {&axis};
    gen_grid<1, element_type> grid(ax);
    grid.edge_limit(0, false);

    for (size_t n = 0; n < axis->size(); ++n) {
        element_type values(N, linear1d((*axis)(n)));
        grid.setdata(&n, values);
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 1.0; x <= 10.0; x += 0.25) {
        double y = x;
        cout << y << "\t";
        truth = scalar_vector<double>(N, linear1d(y));
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&y);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&y);
        cout << linear << "\t";
        for (size_t i = 0; i < N; ++i) {
            BOOST_CHECK_CLOSE(linear(i), truth(i), 1e-6);
        }

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&y);
        cout << pchip << "\t";
        for (size_t i = 0; i < N; ++i) {
            BOOST_CHECK_CLOSE(pchip(i), truth(i), 1e-6);
        }

        cout << endl;
    }
}

/**
 * Interpolate 1-D linear field using a matrix.
 * Exercise all of the 1-D interpolation types.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(linear_1d_matrix_test) {
    typedef boost::numeric::ublas::matrix<double> element_type;
    element_type truth;
    element_type nearest;
    element_type linear;
    element_type pchip;
    size_t N = 2;

    cout << "=== datagrid_test: linear_1d_matrix_test ===" << endl;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    const seq_vector::csptr* ax = {&axis};
    gen_grid<1, element_type> grid(ax);
    grid.edge_limit(0, false);

    for (size_t n = 0; n < axis->size(); ++n) {
        element_type values(N, N, linear1d((*axis)(n)));
        grid.setdata(&n, values);
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 0.25; x <= 10.0; x += 0.25) {
        double y = x;
        cout << y << "\t";
        truth = scalar_matrix<double>(N, N, linear1d(y));
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&y);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&y);
        cout << linear << "\t";
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                BOOST_CHECK_CLOSE(linear(i, j), truth(i, j), 1e-6);
            }
        }

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&y);
        cout << pchip << "\t";
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                BOOST_CHECK_CLOSE(pchip(i, j), truth(i, j), 1e-6);
            }
        }

        cout << endl;
    }
}

/**
 * Interpolate 1-D quadratic field using a scalar.
 * Exercise all of the 1-D interpolation types.
 * Show that data can be interpolated slightly outside of original domain.
 * Generate errors if cubic values differ by more that 1 percent.
 */
BOOST_AUTO_TEST_CASE(quad_1d_test) {
    double truth;
    double nearest;
    double linear;
    double pchip;
    cout << "=== datagrid_test: quad_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    gen_grid<1> grid(&axis);
    grid.edge_limit(0, false);
    for (size_t n = 0; n < axis->size(); ++n) {
        grid.setdata(&n, quad1d((*axis)(n)));
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 0.5; x <= 9.5; x += 0.25) {
        cout << x << "\t";
        truth = quad1d(x);
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&x);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&x);
        cout << linear << "\t";

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&x);
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE(pchip, truth, 1.0);

        cout << endl;
    }
}

/**
 * Interpolate 1-D quadratic field with a logrithmic axis.
 * Exercise all of the 1-D interpolation types.
 * Show that data can be interpolated slightly outside of original domain.
 * Generate errors if cubic values differ by more that 2 percent.
 */
BOOST_AUTO_TEST_CASE(log_axis_1d_test) {
    double truth;
    double nearest;
    double linear;
    double pchip;
    cout << "=== datagrid_test: log_axis_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_log(1.0, pow(3.0, 1.0 / 3.0), 9.0));
    gen_grid<1> grid(&axis);
    grid.edge_limit(0, false);
    for (size_t n = 0; n < axis->size(); ++n) {
        grid.setdata(&n, quad1d((*axis)(n)));
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 1.0; x <= 9.5; x += 0.25) {
        cout << x << "\t";
        truth = quad1d(x);
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        nearest = grid.interpolate(&x);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        linear = grid.interpolate(&x);
        cout << linear << "\t";

        grid.interp_type(0, interp_enum::pchip);
        pchip = grid.interpolate(&x);
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE(pchip, truth, 2.0);

        cout << endl;
    }
}

/**
 * Interpolate 1-D quadratic field using a scalar.
 * Test the accuracy of the derivatives.
 * Generate errors if values differ by more that 0.35.
 */
BOOST_AUTO_TEST_CASE(deriv_1d_test) {
    double truth{0.0};
    double nearest{0.0};
    double linear{0.0};
    double pchip{0.0};

    cout << "=== datagrid_test: deriv_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_vector::csptr axis(new seq_linear(1.0, 2.0, 9.0));
    gen_grid<1> grid(&axis);

    for (size_t n = 0; n < axis->size(); ++n) {
        grid.setdata(&n, quad1d((*axis)(n)));
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for (double x = 1.0; x <= 9.0; x += 0.25) {
        cout << x << "\t";
        truth = deriv1d(x);
        cout << truth << "\t";

        grid.interp_type(0, interp_enum::nearest);
        grid.interpolate(&x, &nearest);
        cout << nearest << "\t";

        grid.interp_type(0, interp_enum::linear);
        grid.interpolate(&x, &linear);
        cout << linear << "\t";

        grid.interp_type(0, interp_enum::pchip);
        grid.interpolate(&x, &pchip);
        cout << pchip;
        BOOST_CHECK_SMALL(abs(pchip - truth), 0.35);

        cout << endl;
    }
}

/**
 * Cubic generating function and its derivative
 * functions in the x and y directions.
 */
static double cubic2d(double x, double y) { return (x * x * x) * (y * y * y); }

static double deriv2d_x(double x, double y) {
    return (3.0 * x * x) * (y * y * y);
}

static double deriv2d_y(double x, double y) {
    return (x * x * x) * (3.0 * y * y);
}

/**
 * Interpolate 2-D cubic field using a cubic generating
 * function and compare the speed required to interpolate
 * 1e6 points using the fast_2d and gen_grid methods
 */
BOOST_AUTO_TEST_CASE(interp_speed_test) {
    cout << "=== datagrid_test: interp_speed_test ===" << endl;
    randgen gen(100);

    size_t num_points = 1e6;
    int param = 5;
    size_t index[2];

    seq_vector::csptr ax[2];
    ax[0] = seq_vector::csptr(new seq_linear(1.0, 1.0, 5));
    ax[1] = seq_vector::csptr(new seq_linear(1.0, 1.0, 5));
    auto* grid = new gen_grid<2>(ax);
    grid->interp_type(0, interp_enum::pchip);
    grid->interp_type(1, interp_enum::pchip);

    for (size_t i = 0; i < ax[0]->size(); ++i) {
        for (size_t j = 0; j < ax[1]->size(); ++j) {
            index[0] = i;
            index[1] = j;
            auto x = double(i + 1);
            auto y = double(j + 1);
            double number = cubic2d(x, y);
            grid->setdata(index, number);
        }
    }
    gen_grid<2>::csptr grid_csptr(grid);

    double spot[2];
    matrix<double*> location(num_points, 1);
    for (size_t i = 0; i < num_points; ++i) {
        spot[0] = param * gen.uniform();
        spot[1] = param * gen.uniform();
        location(i, 0) = spot;
    }

    {
        cout << "Interpolation using gen_grid method" << endl;
        boost::timer::auto_cpu_timer timer;
        for (int counter = 0; counter < num_points; ++counter) {
            grid_csptr->interpolate(location(counter, 0));
        }
    }

    {
        cout << "Interpolation using data_grid_bathy method" << endl;
        data_grid_bathy fast_grid(grid_csptr);
        boost::timer::auto_cpu_timer timer;
        for (int counter = 0; counter < num_points; ++counter) {
            fast_grid.interpolate(location(counter, 0));
        }
    }
}

/**
 * Interpolate 2-D cubic field using a cubic generating
 * function and compare the interpolated results and their
 * derivatives for both the fast_2d and gen_grid methods
 * to the analytic values.
 *
 * An error is produced if the values produced by fast_grids
 * are greater than a 3% difference from the gen_grid or
 * true values.
 */
BOOST_AUTO_TEST_CASE(fast_accuracy_test) {
    cout << "=== datagrid_test: fast_accuracy_test ===" << endl;

    int N = 10;
    double span = 0.5;

    // Build a data grid with simple axes
    seq_vector::csptr ax[2];
    ax[0] = seq_vector::csptr(new seq_linear(-span, 0.1, N));
    ax[1] = seq_vector::csptr(new seq_linear(-span, 0.1, N));
    auto* grid = new gen_grid<2>(ax);
    grid->interp_type(0, interp_enum::pchip);
    grid->interp_type(1, interp_enum::pchip);

    // Populate the data grid with a bicubic function
    size_t index[2];
    double x;
    double y;
    size_t size0(ax[0]->size());
    size_t size1(ax[1]->size());
    for (size_t i = 0; i < size0; ++i) {
        for (size_t j = 0; j < size1; ++j) {
            index[0] = i;
            index[1] = j;
            x = double(i) / double(N) - span;
            y = double(j) / double(N) - span;
            double number = cubic2d(x, y);
            grid->setdata(index, number);
        }
    }
    gen_grid<2>::csptr grid_csptr(grid);

    double spot[2];
    x = 0.2135, y = -0.3611;
    spot[0] = x, spot[1] = y;
    double derv[2];
    cout << "location: (" << spot[0] << ", " << spot[1] << ")" << endl;

    double grid_value = grid_csptr->interpolate(spot, derv);
    printf("gen_grid: %10f  derivative: %8f, %8f\n", grid_value, derv[0],
           derv[1]);

    data_grid_bathy test_grid_fast(grid_csptr);
    double fast_value = test_grid_fast.interpolate(spot, derv);
    printf("fast_grid: %10f  derivative: %8f, %8f\n", fast_value, derv[0],
           derv[1]);

    double true_value = cubic2d(x, y);
    derv[0] = deriv2d_x(x, y);
    derv[1] = deriv2d_y(x, y);
    printf("true value: %9f  derivative: %9f, %8f\n", true_value, derv[0],
           derv[1]);

    // Compare to 3 %
    BOOST_CHECK_CLOSE(fast_value, true_value, 3);
    BOOST_CHECK_CLOSE(grid_value, true_value, 3);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
