
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
# dataset 1
#
my $weight1 =  [ 1,1,1,1,1 ];
my $data1   =  [
        [ 1.1, 2.2, 3.3, 4.4, 5.5, ], 
        [ 3.1, 3.2, 1.3, 2.4, 1.5, ], 
        [ 4.1, 2.2, 0.3, 5.4, 0.5, ], 
        [ 12.1, 2.0, 0.0, 5.0, 0.0, ], 
];
my $mask1 =  [
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
];

#----------
# dataset 2
#
my $weight2 =  [ 1,1 ];
my $data2   =  [
	[ 0.8223, 0.9295 ],
	[ 1.4365, 1.3223 ],
	[ 1.1623, 1.5364 ],
	[ 2.1826, 1.1934 ],
	[ 1.7763, 1.9352 ],
	[ 1.7215, 1.9912 ],
	[ 2.1812, 5.9935 ],
	[ 5.3290, 5.9452 ],
	[ 3.1491, 3.3454 ],
	[ 5.1923, 5.3156 ],
	[ 4.7735, 5.4012 ],
	[ 5.1297, 5.5645 ],
	[ 5.3934, 5.1823 ],
];
my $mask2 =  [
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
];


#------------------------------------------------------
# Tests
# 
my ($result, $output);
my ($i);

#----------
# test dataset 1
#

#--------------[PALcluster]-------
my %params = (

	transpose  =>         0,
	method     =>       'a',
	dist       =>       'e',
	data       =>    $data1,
	mask       =>    $mask1,
	weight     =>  $weight1,
);

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data1) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}


$want = '  0:   2   1   2.600
  1:  -1   0   7.300
  2:   3  -2  21.348
';				test q( $output );


#--------------[PSLcluster]-------
$params{method} = 's';

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data1) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   1   2   2.600
  1:   0  -1   5.800
  2:  -2   3  12.908
';				test q( $output );


#--------------[PCLcluster]-------
$params{method} = 'c';

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data1) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   1   2   2.600
  1:   0  -1   6.650
  2:  -2   3  19.437
';				test q( $output );


#--------------[PMLcluster]-------
$params{method} = 'm';

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data1) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   2   1   2.600
  1:  -1   0   8.800
  2:   3  -2  32.508
';				test q( $output );


#----------
# test dataset 2
#

#--------------[PALcluster]-------
%params = (

	transpose  =>         0,
	method     =>       'a',
	dist       =>       'e',
	data       =>    $data2,
	mask       =>    $mask2,
	weight     =>  $weight2,
);

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data2) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   5   4   0.003
  1:   9  12   0.029
  2:   2   1   0.061
  3:  11  -2   0.070
  4:  -4  10   0.128
  5:   7  -5   0.224
  6:  -3   0   0.254
  7:  -1   3   0.391
  8:  -8  -7   0.532
  9:   8  -9   3.234
 10:  -6   6   4.636
 11: -11 -10  12.741
';				test q( $output );


#print STDERR "\n$want\n\n$output\n";


#--------------[PSLcluster]-------
$params{method} = 's';

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data2) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   4   5   0.003
  1:   9  12   0.029
  2:  11  -2   0.033
  3:   1   2   0.061
  4:  10  -3   0.077
  5:   7  -5   0.092
  6:   0  -4   0.242
  7:  -7  -1   0.246
  8:   3  -8   0.287
  9:  -9   8   1.936
 10: -10  -6   3.432
 11:   6 -11   3.535
';				test q( $output );


#--------------[PCLcluster]-------
$params{method} = 'c';

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data2) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   4   5   0.003
  1:  12   9   0.029
  2:   1   2   0.061
  3:  -2  11   0.063
  4:  10  -4   0.109
  5:  -5   7   0.189
  6:   0  -3   0.239
  7:   3  -1   0.390
  8:  -7  -8   0.382
  9:  -9   8   3.063
 10:   6  -6   4.578
 11: -10 -11  11.536
';				test q( $output );


#--------------[PMLcluster]-------
$params{method} = 'm';

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$data2) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   5   4   0.003
  1:   9  12   0.029
  2:   2   1   0.061
  3:  11  10   0.077
  4:  -2  -4   0.216
  5:  -3   0   0.266
  6:  -5   7   0.302
  7:  -1   3   0.425
  8:  -8  -6   0.968
  9:   8   6   3.975
 10: -10  -7   5.755
 11: -11  -9  22.734
';				test q( $output );


#-------[treecluster on a distance matrix]------------

my $matrix   =  [
        [],
        [ 3.4],
        [ 4.3, 10.1],
        [ 3.7, 11.5,  1.0],
        [ 1.6,  4.1,  3.4,  3.4],
        [10.1, 20.5,  2.5,  2.7,  9.8],
        [ 2.5,  3.7,  3.1,  3.6,  1.1, 10.1],
        [ 3.4,  2.2,  8.8,  8.7,  3.3, 16.6,  2.7],
        [ 2.1,  7.7,  2.7,  1.9,  1.8,  5.7,  3.4,  5.2],
        [ 1.4,  1.7,  9.2,  8.7,  3.4, 16.8,  4.2,  1.3,  5.0],
        [ 2.7,  3.7,  5.5,  5.5,  1.9, 11.5,  2.0,  1.5,  2.1,  3.1],
        [10.0, 19.3,  2.2,  3.7,  9.1,  1.2,  9.3, 15.7,  6.3, 16.0, 11.5]
];

%params = (
	method     =>       's',
	data       =>   $matrix,
);

$result = Algorithm::Cluster::treecluster(%params);

# Make sure that @clusters and @centroids are the right length
$want = scalar(@$matrix) - 1;       test q( scalar @$result );

$output = '';
$i=0;
foreach(@{$result}) {
	$output .= sprintf("%3d: %3d %3d %7.3f\n",$i,$_->[0],$_->[1],$_->[2]);
	++$i
}

$want = '  0:   2   3   1.000
  1:   4   6   1.100
  2:   5  11   1.200
  3:   7   9   1.300
  4:   0  -4   1.400
  5:  -5  10   1.500
  6:  -2  -6   1.600
  7:   1  -7   1.700
  8:   8  -8   1.800
  9:  -1  -9   1.900
 10: -10  -3   2.200
';				test q( $output );

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
