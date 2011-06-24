
my ($last_test,$loaded);

######################### We start with some black magic to print on failure.
use lib '../blib/lib','../blib/arch';

BEGIN { $last_test = 19; $| = 1; print "1..$last_test\n"; }
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
# Data for Tests
# 

#----------
# dataset
#
my $weight =  [ 1,1,1,1,1 ];
my $data   =  [
        [ 1.1, 2.2, 3.3, 4.4, 5.5, ], 
        [ 3.1, 3.2, 1.3, 2.4, 1.5, ], 
        [ 4.1, 2.2, 0.3, 5.4, 0.5, ], 
        [ 12.1, 2.0, 0.0, 5.0, 0.0, ], 
];
my $mask =  [
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
];

#------------------------------------------------------
# Tests
# 
my $matrix;

#----------
# test dataset with transpose==0
#

$matrix = Algorithm::Cluster::distancematrix(
	transpose =>        0,
	dist      =>      'e',
	data      =>    $data,
	mask      =>    $mask,
	weight    =>  $weight,
);


#----------
# Make sure that the length of $matrix matches the length of @data1
$want = scalar @$data;       test q( scalar @$matrix );

#----------
# Test the values in the distance matrix

$want = '  5.800';       test q( sprintf "%7.3f", $matrix->[1]->[0] );
$want = '  8.800';       test q( sprintf "%7.3f", $matrix->[2]->[0] );
$want = '  2.600';       test q( sprintf "%7.3f", $matrix->[2]->[1] );
$want = ' 32.508';       test q( sprintf "%7.3f", $matrix->[3]->[0] );
$want = ' 18.628';       test q( sprintf "%7.3f", $matrix->[3]->[1] );
$want = ' 12.908';       test q( sprintf "%7.3f", $matrix->[3]->[2] );


#----------
# test dataset with transpose==1
#

$matrix = Algorithm::Cluster::distancematrix(
	transpose =>        1,
	dist      =>      'e',
	data      =>    $data,
	mask      =>    $mask,
	weight    =>  $weight,
);

#----------
# Make sure that the length of $matrix matches the length of @data1
$want = scalar @{$data->[0]};       test q( scalar @$matrix );

#----------
# Test the values in the distance matrix


$want = ' 26.71';       test q( sprintf "%6.2f", $matrix->[1]->[0] );
$want = ' 42.23';       test q( sprintf "%6.2f", $matrix->[2]->[0] );
$want = '  3.11';       test q( sprintf "%6.2f", $matrix->[2]->[1] );
$want = ' 15.87';       test q( sprintf "%6.2f", $matrix->[3]->[0] );
$want = '  6.18';       test q( sprintf "%6.2f", $matrix->[3]->[1] );
$want = ' 13.36';       test q( sprintf "%6.2f", $matrix->[3]->[2] );
$want = ' 45.32';       test q( sprintf "%6.2f", $matrix->[4]->[0] );
$want = '  5.17';       test q( sprintf "%6.2f", $matrix->[4]->[1] );
$want = '  1.23';       test q( sprintf "%6.2f", $matrix->[4]->[2] );
$want = ' 12.76';       test q( sprintf "%6.2f", $matrix->[4]->[3] );



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

__END__



