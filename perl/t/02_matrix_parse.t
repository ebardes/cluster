
my ($last_test,$loaded);

######################### We start with some black magic to print on failure.
use lib '../blib/lib','../blib/arch';

BEGIN { $last_test = 13; $| = 1; print "1..$last_test\n"; }
END   { print "not ok 1  Can't load Algorithm::Cluster\n" unless $loaded; }

use Algorithm::Cluster;
no  warnings 'Algorithm::Cluster';

$loaded = 1;
print "ok 1\n";

######################### End of black magic.

sub test;  # Predeclare the test function (defined below)

my $tcounter = 1;
my $want     = '';


#------------------------------------------------------
# Data for Tests, and Tests
# 

# Normal matrix, no errors
my $data2 =  [

	[ 1.1, 1.2 ],
	[ 1.4, 1.3 ],
	[ 1.1, 1.5 ],
	[ 2.0, 1.5 ],
	[ 1.7, 1.9 ],
	[ 1.7, 1.9 ],
	[ 5.7, 5.9 ],
	[ 5.7, 5.9 ],
	[ 3.1, 3.3 ],
	[ 5.4, 5.3 ],
	[ 5.1, 5.5 ],
	[ 5.0, 5.5 ],
	[ 5.1, 5.2 ],
];


# Another normal matrix, no errors
my $data3 =  [

        [ 1.1, 2.2, 3.3, 4.4, 5.5, ], 
        [ 3.1, 3.2, 1.3, 2.4, 1.5, ], 
        [ 4.1, 2.2, 0.3, 5.4, 0.5, ], 
        [ 12.1, 2.0, 0.0, 5.0, 0.0, ], 
];

# Ragged matrix
my $data4 =  [

        [ 91.1, 92.2, 93.3, 94.4, 95.5, ], 
        [ 93.1, 93.2, 91.3, 92.4 ], 
        [ 94.1, 92.2, 90.3 ], 
        [ 12.1, 92.0, 90.0, 95.0, 90.0, ], 
];

my $scalar = 3;
my $scalar_ref = \$scalar;
my $sub_ref = sub {};
my %hash = ();
my $hash_ref = \%hash;

# Matrix with bad cells
my $data5 =  [

        [ 7.1, 7.2, 7.3, 7.4, 7.5, ], 
        [ 7.1, 7.2, 7.3, 7.4, 'snoopy' ], 
        [ 7.1, 7.2, 7.3, $scalar_ref, $sub_ref ], 
        [ 7.1, $hash_ref, 7.0, 7.0, 7.0, ], 
];

# Matrix with a bad row
my $data6 =  [

        [ 23.1, 23.2, 23.3, 23.4, 23.5, ], 
        $scalar_ref,
        [ 23.1, 23.0, 23.0, 23.0, 23.0, ], 
];


# Various references that do not point to matrices at all
my $data7 = $scalar_ref;
my $data8 = $sub_ref;
my $data9 = $hash_ref;
my $data10 = [];
my $data11 = [undef()];
my $data12 = [[]];
my $data13 = [[ undef() ]];


$want = 'Row   0:      1.10    1.20
Row   1:      1.40    1.30
Row   2:      1.10    1.50
Row   3:      2.00    1.50
Row   4:      1.70    1.90
Row   5:      1.70    1.90
Row   6:      5.70    5.90
Row   7:      5.70    5.90
Row   8:      3.10    3.30
Row   9:      5.40    5.30
Row  10:      5.10    5.50
Row  11:      5.00    5.50
Row  12:      5.10    5.20
';
			test q( Algorithm::Cluster::readformat( $data2 ););

$want = 'Row   0:      1.10    2.20    3.30    4.40    5.50
Row   1:      3.10    3.20    1.30    2.40    1.50
Row   2:      4.10    2.20    0.30    5.40    0.50
Row   3:     12.10    2.00    0.00    5.00    0.00
';       test q( Algorithm::Cluster::readformat( $data3 ););

$want = 'undef';
			test q( Algorithm::Cluster::readformat( $data4 ););
$want = 'undef';
			test q( Algorithm::Cluster::readformat( $data5 ););
$want = 'undef';
			test q( Algorithm::Cluster::readformat( $data6 ););

$want = 'undef';       test q( Algorithm::Cluster::readformat( $data7 ););
$want = 'undef';       test q( Algorithm::Cluster::readformat( $data8 ););
$want = 'undef';       test q( Algorithm::Cluster::readformat( $data9 ););
$want = 'undef';       test q( Algorithm::Cluster::readformat( $data10 ););
$want = 'undef';       test q( Algorithm::Cluster::readformat( $data11 ););
$want = 'undef';       test q( Algorithm::Cluster::readformat( $data12 ););
$want = 'undef';       test q( Algorithm::Cluster::readformat( $data13 ););

#------------------------------------------------------
# Test function
# 
sub test {
	$tcounter++;

	my $string = shift;
	my $ret = eval $string;
	$ret = 'undef' if not defined $ret;

	if("$ret" =~ /^$want$/sm) {

		print "ok $tcounter\n";

	} else {
		print "not ok $tcounter\n",
		"   -- '$string' returned '$ret'\n", 
		"   -- expected =~ /$want/\n"
	}
}


