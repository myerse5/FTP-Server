#!/usr/bin/perl

###############################################################################
# Author: Evan Myers
# Date: June 2014
#
# FTP Config v0.1
#
# A perl script which uses a GUI to make setting up or changing the FTP
# Configuration File 'ftp.conf' more straightforward for the user.
#
# TODO list:
#   -Add some form of feedback to the user to show that the 'Configure' button
#    has applied the changes or failed to do so.
#   -Add a check for non digit characters in the port entry for the 'Configure'
#    button signal.
#   -Use `ifconfig` instead of `ip` so that systems other than linux can use
#    this script. Look into "Net::Ifconfig::Wrapper" perhaps.
#   -Add the [-d] option as advertised using argv or getopt, then tie the clear
#    button to it.
#   -Merge the 'localiface' and 'globaliface' into a single 2 column model.
#   -Memory leak(s) when you find out more about perl and gtk. Consider looking
#    into http://search.cpan.org/~ni-s/Devel-Leak-0.03/Leak.pm
#   -optimize (not too horribly important right now though).
#   -Add a SIGHUP option/button?
###############################################################################

use strict;
use warnings;
use Tie::File;
use Glib qw/TRUE FALSE/;
use Gtk2::Pango;
use Gtk2 '-init';

use constant CONFIG_FILE => "ftp.conf";    # FTP configuration file
# Replace the lines in the config file that contain these constants with the
# new values.
use constant IFACE_CONFIG => "INTERFACE_CONFIG";
use constant NAT_MODE_CONFIG => "NAT_MODE_CONFIG";
use constant PORT_CONFIG => "DEFAULT_PORT_CONFIG";

# True if NAT radio button is selected in the interface frame. 
my $NAT_MODE = FALSE;

###############################################################################
# When the checkbox on the "Interface Selection" frame is toggled, show or
# hide the combo box that allows the user to select from a list of interfaces
# that are compatible with the current selection. Callback.
###############################################################################
sub iface_select_toggle
{
    my ($checkBtn, $comboBox) = @_;

    if ($checkBtn->get_active) {
	$comboBox->show;
    } else {
	$comboBox->hide;
    }
}


###############################################################################
# Set the entry on the "Port Selection" frame when appropriate, and change the
# sensitivity of the entry when the radio buttons on the "Port Selection" frame
# are toggled. Callback.
###############################################################################
sub port_toggled
{
    my ($radioBtn, $entry) = @_;

    if ($radioBtn->get_active) {
	$entry->sensitive (TRUE);
    } else {
	$entry->sensitive (FALSE);
	$entry->set_text ("21");
    }
}


###############################################################################
# Whenever the entry in the "Port Selection" frame is changed, determine if the
# entry is a positive integer. If it is not a positive integer, display the
# error message below the previously mentioned entry widget. Callback.
###############################################################################
sub port_error
{
    my ($entry, $label) = @_;
    my $string;

    $string = $entry->get_text;

    # Show the error message if the entry contains anything that is not a
    # digit, or if the string is empty.
    if (($string =~ m/\D+/) || ($string =~ m/^$/)) {
	$label->show;
    } else {
	$label->hide;
    }
}


###############################################################################
# When the radio buttons in the frame "Interface Selection" are toggled,
# change the descriptive message to the new selection, and change the list
# of the interfaces that the user may choose to the new compatible list.
# Callback.
###############################################################################
sub iface_radio_toggle
{
    my ($radioBtn, $argsRef) = @_;
    my @args = @$argsRef;

    my ($label, $mesg, $comboBox, $model) = @args;

    if ($radioBtn->get_active) {
	$comboBox->set_model ($model);
	$comboBox->set_active (0);
	$label->set_label ($mesg);

	if ($radioBtn->get_label =~ m/NAT/) {
	    $NAT_MODE = TRUE;
	} else {
	    $NAT_MODE = FALSE;
	}
    }
}


###############################################################################
# In the "Interface selection" frame there are radio buttons that allow the
# user to choose between loopback interfaces or global interfaces. Create two
# models (views) that will be displayed in the combo box of the "Interface
# Selection" frame that can be displayed depending on which type of interface
# the user wishes to select.
###############################################################################
sub create_iface_models
{
    # localRef is a reference to the local interface model.
    my ($localRef, $globalRef) = @_;

    my $iface;
    my $details;
    my $i = 0;
    my $j = 0;
    my @ipAddrShow = `ip addr show`;


    $$localRef = Gtk2::ListStore->new ("Glib::String");
    $$globalRef = Gtk2::ListStore->new ("Glib::String");

    foreach my $line (@ipAddrShow) {
	if ($line =~ m/^[0-9]+:\s+([^:]*):\s<(.*)>.*/) {
	    $iface = $1;
	    $details = $2;

	    if ($details =~ m/LOOPBACK/) {
		$$localRef->set ($$localRef->append, $i++, $iface);
	    } else {
		$$globalRef->set ($$globalRef->append, $j++, $iface);
	    }
	}
    }
}


###############################################################################
# Display what the program does and who should use it to the user in the
# topmost frame in the GUI.
###############################################################################
sub create_about_frame
{
    my $windowTable = shift;
    my $table;
    my $label;
    my $frame;

    my $mesg =
	"Select the network interface and port to be used by the FTP server. " .
	"Press the [Configure] button in the bottom right hand corner of " .
	"this screen to modify the file 'ftp.conf' for your selection\n\n" .
	"Developers may wish to run this script with the option [-d] for " .
	"more options.";

    $frame = Gtk2::Frame->new ("About");
    $windowTable->attach ($frame, 0, 1, 0, 1, 'fill', 'shrink', 0, 0);
    $frame->show;

    $table = Gtk2::Table->new (1, 1, FALSE);
    $frame->add ($table);
    $table->set_border_width (10);
    $table->show;

    $label = Gtk2::Label->new ($mesg);
    $table->attach_defaults ($label, 0, 1, 0, 1);
    $label->set_line_wrap (TRUE);
    $label->show;
}


###############################################################################
# Display a frame, which asks the user what type of network interface they
# would like to use. Update this description after.
#
# Returns: The combo box with the interface to be used selected.
###############################################################################
sub create_iface_frame
{
    my $windowTable = shift;
    my $radioBtn;
    my $frame;
    my $frameTable;
    my $radioTable;
    my $label;
    my $checkBtn;
    my $comboBox;
    my $localModel;
    my $globalModel;
    my @labels;
    my @mesg;

    # Labels to appear on each radio button.
    $labels[0] = "default";
    $labels[1] = "NAT";
    $labels[2] = "loopback";

    # Messages to appear below the radio buttons. Only one message will be
    # present on the screen at any moment. The message that is displayed will
    # be determined by which radio button is toggled.
    $mesg[0] = 
	"Use the selected interface and display the IP address as it appears " .
	"in the server console."; 
    $mesg[1] =
	"Use the selected interface and attempt to display the globally " .
	"reachable IP address in the server console. Suitable for when " .
	"attempting to make the server reachable on the internet and your " .
	"machine is behind a NAT router.";
    $mesg[2] = 
	"Loopback - The server can only be reached on this machine. Suitable " .
	"for when testing the server when there is no network to connect with.";

    # All widgets created in this function will appear in this frame.
    $frame = Gtk2::Frame->new ("Interface Selection");
    $windowTable->attach ($frame, 0, 1, 1, 2, 'fill', 'shrink', 0, 0);
    $frame->show;

    # A table to organize the widgets in the frame.
    $frameTable = Gtk2::Table->new (1, 3, FALSE);
    $frame->add ($frameTable);
    $frameTable->set_border_width (8);
    $frameTable->set_row_spacings (8);
    $frameTable->show;

    # A label to inform the user on what their current selection does.
    $label = Gtk2::Label->new ($mesg[0]);
    $frameTable->attach_defaults ($label, 0, 1, 2, 3);
    $label->set_alignment (0, 0.5);
    $label->set_line_wrap (TRUE);
    # Required for a visually appealing line wrap.
    $label->set_size_request (405, -1);
    $label->show;

    # This combo box allows the user to choose which interface to use from
    # a filtered list of available interfaces. The filter which is applied
    # either shows loopback interfaces, or non loopback interfaces.
    $comboBox = Gtk2::ComboBox->new_text ();
    $frameTable->attach ($comboBox, 0, 1, 1, 2, 'fill', 'fill', 8, 0);

    # Create the interface models to be displayed in the combo box.
    create_iface_models (\$localModel, \$globalModel);
    $comboBox->set_model ($globalModel);
    $comboBox->set_active (0);

    # A table which contains all the radio buttons. This was created to easily
    # show and hide all radio buttons.
    $radioTable = Gtk2::Table->new (1, 4, FALSE);
    $frameTable->attach_defaults ($radioTable, 0, 1, 0, 1);
    $radioTable->show;

    $radioBtn = Gtk2::RadioButton->new ($radioBtn, $labels[0]);
    $radioTable->attach_defaults ($radioBtn, 0, 1, 0, 1);
    $radioBtn->show;
    $radioBtn->signal_connect (toggled => \&iface_radio_toggle,
			       [$label, $mesg[0], $comboBox, $globalModel]);

    $radioBtn = Gtk2::RadioButton->new ($radioBtn, $labels[1]);
    $radioTable->attach_defaults ($radioBtn, 0, 1, 1, 2);
    $radioBtn->show;
    $radioBtn->signal_connect (toggled => \&iface_radio_toggle,
			       [$label, $mesg[1], $comboBox, $globalModel]);

    $radioBtn = Gtk2::RadioButton->new ($radioBtn, $labels[2]);
    $radioTable->attach_defaults ($radioBtn, 0, 1, 2, 3);
    $radioBtn->show;
    $radioBtn->signal_connect (toggled => \&iface_radio_toggle,
			       [$label, $mesg[2], $comboBox, $localModel]);

    # Determines if the user wants the script to auto configure, or if the user
    # wants to choose a specific interface.
    $checkBtn = Gtk2::CheckButton->new ("Let me choose the interface");
    $radioTable->attach ($checkBtn, 0, 1, 3, 4, 'fill', 'fill', 20, 0);
    $checkBtn->show;

    $checkBtn->signal_connect (toggled => \&iface_select_toggle, $comboBox);

    return $comboBox;
}


###############################################################################
# Allow the user to specify a port or use the default. This function does not
# currently support services (only integer valued port can be used).
#
# Returns: The entry with the port to be used entered in it.
###############################################################################
sub create_port_frame
{
    my $windowTable = shift;
    my $frame;
    my $radioBtn;
    my $entry;
    my $table;
    my $label;

    $frame = Gtk2::Frame->new ("Port Selection");
    $windowTable->attach ($frame, 0, 1, 2, 3, 'fill', 'shrink', 0, 0);
    $frame->show;

    $table = Gtk2::Table->new (1, 4, FALSE);
    $frame->add ($table);
    $table->set_border_width (8);
    $table->show;

    $radioBtn = Gtk2::RadioButton->new (undef, "Choose standard FTP port");
    $table->attach_defaults ($radioBtn, 0, 1, 0, 1);
    $radioBtn->show;

    $radioBtn = Gtk2::RadioButton->new ($radioBtn, "Let me choose the port");
    $table->attach_defaults ($radioBtn, 0, 1, 1, 2);
    $radioBtn->show;

    $entry = Gtk2::Entry->new;
    $table->attach_defaults ($entry, 0, 1, 2, 3);
    $entry->set_text ("21");
    $entry->sensitive (FALSE);
    $entry->show;

    $radioBtn->signal_connect (toggled => \&port_toggled, $entry);

    $label = Gtk2::Label->new ("Error: Port must be a positive integer");
    $table->attach ($label, 0, 1, 3, 4, 'fill', 'shrink', 8, 8);
    $label->set_alignment (0, 0.5);

    $entry->signal_connect (changed => \&port_error, $label);

    return $entry;
}


###############################################################################
# Creates the area which contains the "Configure" button. The "Clear ftp.conf"
# button is created when the script is run with the [-d] option.
###############################################################################
sub create_accept_area
{
    my ($windowTable, $comboBox, $entry) = @_;

    my $btn;
    my $label;
    my $align;

    $align = Gtk2::Alignment->new (1, 0.5, 0, 1);
    $windowTable->attach ($align, 0, 1, 3, 4, 'fill', 'shrink', 0, 0);
    $align->show;

    $btn = Gtk2::Button->new;
    $align->add ($btn);
    $btn->show;

    $label = Gtk2::Label->new;
    $btn->add ($label);
    $label->set_markup ('<span weight="bold">Configure</span>');
    $label->show;

    $btn->signal_connect (clicked => \&submit_changes, [$comboBox, $entry]);

    $align = Gtk2::Alignment->new (0, 0.5, 0, 1);
    $windowTable->attach ($align, 0, 1, 3, 4, 'fill', 'shrink', 0, 0);
    $align->show;

    $btn = Gtk2::Button->new ("Clear ftp.conf settings");
    $align->add ($btn);
    $btn->show;
}


###############################################################################
#
###############################################################################
sub submit_changes
{
    my ($btn, $argRef) = @_;
    my ($comboBox, $entry) = @$argRef;

    my @file;
    my $line;
    my $replaced = 0;
    my $lineNumber = 0;
    my $iface = $comboBox->get_active_text;
    my $port = $entry->get_text;
    my $nat = $NAT_MODE;
    my $ifaceChanged = FALSE;
    my $portChanged = FALSE;
    my $natChanged = FALSE;

    tie @file, 'Tie::File', CONFIG_FILE
	or die "error: failed to open " .  CONFIG_FILE . ": $!";

    my $i = 0;
    foreach $line (@file) {
	# Skip to the next line if comment or starts with whitespace.
	if (($line =~ m/^#.*/) || ($line =~ m/^\s.*/)) {
	    next;
	}

	# Replace interface, port, or nat lines in the config file with the
	# new values.
	if ($line =~ m/^(${\IFACE_CONFIG}).*/) {
	    $line = "$1 $iface";
	    $file[$lineNumber] = $line;
	    $replaced++;
	    $ifaceChanged = TRUE;
	} elsif ($line =~ m/^(${\PORT_CONFIG}).*/) {
	    $line = "$1 $port";
	    $file[$lineNumber] = $line;
	    $replaced++;
	    $portChanged = TRUE;
	} elsif ($line =~ m/^(${\NAT_MODE_CONFIG}).*/) {
	    if ($nat == TRUE) {
		$line = "$1 TRUE";
	    } else {
		$line = "$1 FALSE";
	    }
	    $file[$lineNumber] = $line;
	    $replaced++;
	    $natChanged = TRUE;
	}
    } continue {
	$lineNumber++;
	$i++;
	# Stop comparing lines to replace when all values have been replaced.
	if ($replaced == 3) {
	    last;
	}
    }

    # If the config file was missing one of the network settings, append the
    # setting to the end of the file.
    while ($replaced < 3) {
	if ($ifaceChanged == FALSE) {
	    $file[$lineNumber++] = IFACE_CONFIG . " $iface\n";
	    $ifaceChanged = TRUE;
	    $replaced++;
	} elsif ($portChanged == FALSE) {
	    $file[$lineNumber++] = PORT_CONFIG . " $port\n";
	    $portChanged = TRUE;
	    $replaced++;
	} else {
	    if ($nat == TRUE) {
		$file[$lineNumber++] = NAT_MODE_CONFIG . " TRUE";
	    } else {
		$file[$lineNumber++] = NAT_MODE_CONFIG . " FALSE";
	    }
	}
    }

    untie @file;        # Close the config file.
}

###############################################################################
# main
###############################################################################
my $window;
my $windowTable;
my $comboBox;
my $entry;
my @file;


unless (-e CONFIG_FILE) {
    print "error: could not find the file '" . CONFIG_FILE . "'\n";
    exit -1;
}



# Create the window that will contain all other widgets.
$window = Gtk2::Window->new ('toplevel');
$window->set_border_width (10);
$window->set_title ("FTP Network Configuration");

# Connect the appropriate events to allow the program to terminate when the
# window is closed.
$window->signal_connect (delete_event => sub { return FALSE; });
$window->signal_connect (destroy => sub { Gtk2->main_quit; });

# Create a table that will arrange all the major components (frames or areas)
# of the window vertically.
$windowTable = Gtk2::Table->new (1, 4, FALSE);
$window->add ($windowTable);
$windowTable->set_row_spacings (10);
$windowTable->show;

# Create each frame or area with these subroutine calls.
create_about_frame ($windowTable);

# The returns from these functions are past to the 'create_accept_area'
# function, where the 'Configure' button can read the values stored in them.
$comboBox = create_iface_frame ($windowTable);
$entry = create_port_frame ($windowTable);
create_accept_area ($windowTable, $comboBox, $entry);

# Show the window and run the GUI.
$window->show;
Gtk2->main;

