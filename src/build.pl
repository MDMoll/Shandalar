use strict;
use warnings;
use File::Path qw(make_path);
use File::Spec;
use File::Copy;
use Getopt::Long qw(GetOptions);

sub usage {
    my ($exit_code) = @_;
    print <<"USAGE";
Usage: perl build.pl [options] [card_function_name]

Adds an extern/jump entry for card_function_name, runs make, and optionally
copies ManalinkEh.dll to explicitly requested destinations.

Options:
  --dry-run              Print planned actions without editing, building, or copying.
  --out-dir DIR          Copy ManalinkEh.dll into DIR after a successful build.
  --copy-to PATH         Copy ManalinkEh.dll to PATH after a successful build.
                         If PATH does not end in .dll, it is treated as a directory.
                         May be provided more than once.
  --legacy-copy-targets  Also copy to the historical c:\\magic2k destinations.
  -h, --help             Show this help.

This script no longer copies to c:\\magic2k unless --legacy-copy-targets or
--copy-to is supplied.
USAGE
    exit $exit_code;
}

my $dry_run = 0;
my $out_dir;
my @copy_to;
my $legacy_copy_targets = 0;
my $help = 0;

GetOptions(
    'dry-run'             => \$dry_run,
    'out-dir=s'           => \$out_dir,
    'copy-to=s@'          => \@copy_to,
    'legacy-copy-targets' => \$legacy_copy_targets,
    'help|h'              => \$help,
) or usage(2);

usage(0) if $help;
usage(2) if @ARGV > 1;

sub destination_for {
    my ($path) = @_;
    return $path if $path =~ /\.dll\z/i;
    return File::Spec->catfile($path, 'ManalinkEh.dll');
}

sub add_card_function {
    my ($card_name, $dry_run) = @_;
    my $new_function = "_card_$card_name";
    my $skip = 0;
    my $extern = 0;
    my $jmp = 0;
    my $hex;

    open my $fh, '<', 'ManalinkEh.asm' or die "Could not open ManalinkEh.asm: $!";
    my @lines;
    while (my $line = <$fh>) {
        if ($line =~ /\Q$new_function\E/ and not $skip) {
            print "You already added that card.\n";
            $skip = 1;
        }
        if ($line =~ /extern/) {
            $extern = 1;
        }
        if ($extern and $line !~ /extern/ and not $skip) {
            push @lines, "extern $new_function\n";
            $extern = 0;
        }
        if ($line =~ /jmp near/) {
            $jmp = 1;
            ($hex) = $line =~ /;\s+([A-Z0-9]+)/i;
        }
        if ($jmp and $line !~ /jmp near/ and not $skip) {
            die "Could not determine jump offset before adding $new_function\n" unless defined $hex;
            my $a = eval("0x$hex") + 5;
            my $new_hex = sprintf "%x", $a;
            push @lines, "  jmp near $new_function     ; $new_hex\n";
            $jmp = 0;
            print "Adding $new_function at $new_hex\n";
        }
        push @lines, $line;
    }
    close $fh;

    if ($skip) {
        print "No ManalinkEh.asm edit needed.\n";
        return;
    }

    if ($dry_run) {
        print "Dry run: would rewrite ManalinkEh.asm with $new_function.\n";
        return;
    }

    open $fh, '>', 'ManalinkEh.asm' or die "Could not write ManalinkEh.asm: $!";
    print {$fh} @lines;
    close $fh;
}

# Open ManalinkEh.asm and add in the new card / function
my $new_function = $ARGV[0];
if ($new_function) {
    add_card_function($new_function, $dry_run);
}

# Run make
if ($dry_run) {
    print "Dry run: would run make.\n";
}
else {
    system('make') == 0 or die "make failed with exit code " . ($? >> 8) . "\n";
}

# Copy the new dll to the magic directory
# copy('ManalinkEh.dll','c:\magic\ManalinkEh.dll') or die "Copy failed: $!";
if (defined $out_dir) {
    push @copy_to, destination_for($out_dir);
}

if ($legacy_copy_targets) {
    push @copy_to, 'c:\magic2k\ManalinkEh.dll';
    push @copy_to, 'c:\magic2k\zips\ManalinkEh.dll';
}

if (!@copy_to) {
    print "No copy targets supplied; leaving ManalinkEh.dll in the build directory.\n";
}

for my $target (@copy_to) {
    my $destination = destination_for($target);
    if ($dry_run) {
        print "Dry run: would copy ManalinkEh.dll to $destination.\n";
        next;
    }
    my (undef, $directory) = File::Spec->splitpath($destination);
    make_path($directory) if defined $directory && length $directory && !-d $directory;
    copy('ManalinkEh.dll', $destination) or die "Copy to $destination failed: $!";
}

#clean up
#`make clean`;
