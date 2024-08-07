#!/usr/bin/perl 

#*
#* GnuDialer - Complete, free predictive dialer
#*
#* Complete, free predictive dialer for contact centers.
#*
#* Copyright (C) 2006, GnuDialer Project
#*
#* Richard Lyman <richard@dynx.net>
#*
#* This program is free software, distributed under the terms of
#* the GNU General Public License.
#*

print "Content-type: text/html\n\n";

use CGI::Carp qw(fatalsToBrowser carpout);
use CGI qw(:standard);

use DBI;
$drh = DBI->install_driver('mysql') or die("Unable to install driver");
$dbh = DBI->connect("DBI:mysql:dialer",'dialer','1234') or die("Unable to connect");
   
$testmode = 0;

###################
#my %param = map { $_, param($_) }  param();
&fetchit;
###################

#$myurl="http://192.168.55.105";
$myurl="";

$mycgi = $ENV{'SCRIPT_NAME'};

if ($FORM{'fs'}) {
   $fs = $FORM{'fs'};
   $fstate = $FORM{'fstate'};
   if ($fs) {
      $fstate = $fs;
   }
}

$referer = $ENV{'HTTP_REFER'};

$table = $FORM{'table'};

$leadid = $FORM{'leadid'};
$phone = $FORM{'phone'};
$name = uc $FORM{'name'};
$queue = uc $FORM{'queue'};

#$name =~ s/(\w+)/\u\L$1/g;

($sec,$min,$hour,$mday,$month,$year,$wday) =
(localtime(time))[0,1,2,3,4,5,6];
$mydatetimestamp = sprintf("%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",
$year+1900, $month+1, $mday, $hour, $min, $sec);


if ($fstate eq 'errormsg') {&myerror; }
elsif ($fstate eq 'lookup') {&what_dbs; &base; }
elsif ($fstate eq 'edit') {&edit_record; }
elsif ($fstate eq 'update') {&update_record; &clear_params; &base;}
elsif ($fstate eq 'env') {&env; &base;}
elsif ($fstate eq 'fixdupes') {&fix_dupes; }
else {&base; }

$dbh->disconnect;
#####################################################################################

################################################################################################
sub base {
print <<EOT_MARKER;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>

<head>
   <meta http-equiv="Expire" content="now">
   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
   <meta name="description" content="none">
   <meta name="keywords" content="none">
   <meta name="GENERATOR" content="Mozilla/4.7 [en] (Win98; I) [Netscape]">
   <meta name="robots" content="noindex,nofollow">
   <title>Lookup Util</title>
</head>

<body>
<center>
EOT_MARKER

#      print "<tr><td>$database</td><td>$table</td><td>$leadid</td><td>$phone</td><td>$name</td> </tr>";
      
print <<EOT_MARKER;

<form method="POST" action="$mycgi">
<table BORDER=0 WIDTH="400" CELLSPACING=0 CELLPADDING=2 COLS=2 BGCOLOR="#CCFFFF">
<tr><td width=70><div align=right>LeadID:</div></td><td><input name="leadid" value="$leadid" size=10></td></tr>
<tr><td width=70><div align=right>Phone:</div></td><td><input name="phone" value="$phone" size=15></td></tr>
<tr><td width=70><div align=right>Name:</div></td><td><input name="name" value="$name" size=40></td></tr>

</td>
<tr>

<td width=70><div align=right>

</div></td>
<td>
<input type=submit value="Lookup">
</td>
</tr>
<tr><td></td><td>
Fill-in at least 1 box (Limit 10 per table shown)</td></tr>
</td></tr>
</center>
</table>
</form>
EOT_MARKER

   what_dbs();

print <<EOT_MARKER;

</body>

</html>
EOT_MARKER

}
######################################################################################
sub fix_dupes {

   $mystring = "UPDATE " . $queue . " SET phone = substring( phone, 2, 10 )  WHERE left( phone, 1 ) = '1' AND length( phone ) =11";
   $sth = $dbh->prepare("$mystring");
   $sth->execute or die("Unable to excute query");


   $mystring = "SELECT id,phone,count(1) FROM " . $queue . " GROUP BY phone HAVING count(1) > 1";
   $sth = $dbh->prepare("$mystring");
   $sth->execute or die("Unable to excute query");

   $i=1;

   while(@record = $sth->fetchrow_array) {
        $mywritestring = "update " . $queue . " set disposition = '-8' WHERE id='" . $record[0] . "'";
        $sth1 = $dbh->prepare("$mywritestring");
        $sth1->execute or die("Unable to excute query");
        $i++;
   }
}
#######################################################################################
sub clear_params {
  $FORM{'leadid'} = "";
  $FORM{'phone'} = "";
  $FORM{'name'} = "";

  $leadid = $FORM{'leadid'};
  $phone = $FORM{'phone'};
  $name = uc $FORM{'name'};
  
}
#######################################################################################
sub edit_record {
print <<EOT_MARKER;
   
  <form method="GET" action="$mycgi">
  <input type = "HIDDEN" name = "fs" value = "update">
  <input type = "HIDDEN" name = "table" value = $table>
  <input type = "HIDDEN" name = "leadid" value = $leadid>
  
  <table BORDER=0 WIDTH="400" CELLSPACING=0 CELLPADDING=2 COLS=2 BGCOLOR="#CCFFFF">

EOT_MARKER
  
   $mystring = "SELECT * FROM " . $table . " WHERE id=$leadid ";
   $sth = $dbh->prepare("$mystring");
   $sth->execute or die("Unable to excute query");
   
   $i=1;
   while (@data=$sth->fetchrow_array()) {       
        foreach $col (@data) {
                if ($sth->{NAME}->[$i-1] ne "id" && $sth->{NAME}->[$i-1] ne "lastupdated" && $sth->{NAME}->[$i-1] ne "wdayl" && $sth->{NAME}->[$i-1] ne "wdayh" && $sth->{NAME}->[$i-1] ne "satl" && $sth->{NAME}->[$i-1] ne "sath" && $sth->{NAME}->[$i-1] ne "sunl" && $sth->{NAME}->[$i-1] ne "sunh" && $sth->{NAME}->[$i-1] ne "holl" && $sth->{NAME}->[$i-1] ne "holh" && $sth->{NAME}->[$i-1] ne "tzl" && $sth->{NAME}->[$i-1] ne "tzh") {
                	if ($sth->{NAME}->[$i-1] eq "disposition") {
                    		#print "<tr><td><div align=right>" . $sth->{NAME}->[$i-1] . ":</div></td><td>\n<input name=" . $sth->{NAME}->[$i-1] . " value=\"" . $col . "\"></td></tr>\n";
                    		print "<tr><td><div align=right>" . $sth->{NAME}->[$i-1] . ":</div></td><td>\n";

				print "<select name=" . $sth->{NAME}->[$i-1] . " size=1 value=" . $col . ">";

				    if ($col == "-8") { print "<option SELECTED>-8 System Burned<br>\n";
				      } else { print "<option>-8 System Burned<br>\n"; }
				    if ($col == "-7") { print "<option SELECTED>-7 System Disconnect<br>\n";
				      } else { print "<option>-7 System Disconnected<br>\n"; }
				    if ($col == "-6") { print "<option SELECTED>-6 System Fax Machine<br>\n";
				      } else { print "<option>-6 System Fax Machine<br>\n"; }
				    if ($col == "-5") { print "<option SELECTED>-5 System Congestion<br>\n";
				      } else { print "<option>-5 System Congestion<br>\n"; }
				    if ($col == "-4") { print "<option SELECTED>-4 System Busy<br>\n";
				      } else { print "<option>-4 System Busy<br>\n"; }
				    if ($col == "-3") { print "<option SELECTED>-3 System Answering Mach<br>\n";
				      } else { print "<option>-3 System Answering Mach<br>\n"; }
				    if ($col == "-2") { print "<option SELECTED>-2 System No Answer<br>\n";
				      } else { print "<option>-2 System No Answer<br>\n"; }
				    if ($col == "-1") { print "<option SELECTED>-1 System Reserved<br>\n";
				      } else { print "<option>-1 System Reserved<br>\n"; }


				    if ($col == "0") { print "<option SELECTED>0 Callback<br>\n";
				      } else { print "<option>0 Callback<br>\n"; }
				    if ($col == "1") { print "<option SELECTED>1 Fresh<br>\n";
				      } else { print "<option>1 Fresh<br>\n"; }
				    if ($col == "2") { print "<option SELECTED>2 No Answer<br>\n";
				      } else { print "<option>2 No Answer<br>\n"; }
				    if ($col == "3") { print "<option SELECTED>3 Answering Machine<br>\n";
				      } else { print "<option>3 Answering Machine<br>\n"; }
				    if ($col == "4") { print "<option SELECTED>4 Busy<br>\n";
				      } else { print "<option>4 Busy<br>\n"; }
				    if ($col == "5") { print "<option SELECTED>5 Congestion<br>\n";
				      } else { print "<option>5 Congestion<br>\n"; }
				    if ($col == "6") { print "<option SELECTED>6 Fax Machine<br>\n";
				      } else { print "<option>6 Fax Machine<br>\n"; }
				    if ($col == "7") { print "<option SELECTED>7 Disconnected<br>\n";
				      } else { print "<option>7 Disconnected<br>\n"; }
				    if ($col == "8") { print "<option SELECTED>8 Do Not Call<br>\n";
				      } else { print "<option>8 Do Not Call<br>\n"; }
				    if ($col == "9") { print "<option SELECTED>9 Invalid<br>\n";
				      } else { print "<option>9 Invalid<br>\n"; }
				    if ($col == "10") { print "<option SELECTED>10 Other<br>\n";
				      } else { print "<option>10 Other<br>\n"; }
				    if ($col == "11") { print "<option SELECTED>11 No Sale<br>\n";
				      } else { print "<option>11 No Sale<br>\n"; }
				    if ($col == "12") { print "<option SELECTED>12 Sale<br>\n";
				      } else { print "<option>12 Sale<br>\n"; }
          
				    print "</select>";

				    print "</td></tr>";
			} else {
                    		print "<tr><td><div align=right>" . $sth->{NAME}->[$i-1] . ":</div></td><td>\n<input name=" . $sth->{NAME}->[$i-1] . " value=\"" . $col . "\"></td></tr>\n";
			}
                }
                $i++;
        }
   }


print <<EOT_MARKER;
</td>
<tr>

<td><div align=right><input type=submit value="Update"></div></td>
<td> </td></tr>

</center>
</table>
</form>
EOT_MARKER
}
#######################################################################################
sub update_record {
print <<EOT_MARKER;
  <center>
  <table BORDER=0 WIDTH="400" CELLSPACING=0 CELLPADDING=2 COLS=2 BGCOLOR="#CCFFFF">

EOT_MARKER
  
   $mystring = "UPDATE " . $table . " SET ";
   
   foreach $key (sort keys %FORM)
   {
      if ($key ne "fs" && $key ne "table" && $key ne "leadid" && $key ne "lastupdated") { 
          $mystring = $mystring . $key . "='" . $FORM{$key} . "',";
          if ($testmode) {
              print qq|<tr><th align="right">$key:</th><td>$FORM{$key}</td></tr>\n|;
          }     
      } 
   }

   $mystring =~ s/\,$//;   
   $mystring = $mystring . " WHERE id='" . $leadid . "'";
   
   if ($testmode) {
       print "$mystring";
   }
   
   $sth = $dbh->prepare("$mystring");
   $sth->execute or die("Unable to excute query");
      

print <<EOT_MARKER;

<tr><td>Record Updated!</td></tr>
</table>
</center>
EOT_MARKER
}
####################################################################################################
sub what_dbs {
   my $ret = 0;
   #$dbh = DBI->connect("DBI:mysql:dialer",'dialer','1234') or die("Unable to connect");
   my $sth = $dbh->prepare("SHOW TABLES");

   $sth->execute or print "query: what_dbs failed";

   while(@table = $sth->fetchrow_array) {
      if ($table[0] ne 'cdr' && $table[0] ne 'CDR' &&$table[0] ne 'dnc' && $table[0] ne 'DNC') {
        search_table($table[0]);
      }
   }

   $sth->finish;
   #$dbh->disconnect;
   return $ret;
}
###############################
sub search_table {
   local($thetable) = @_;


   print "<tr><td border=1 BGCOLOR=#CCFFFF> Table: $thetable </td></tr>";
   
   if ($leadid ne "" || $phone ne "" || $name ne "") {
   	$mystring = "SELECT * FROM " . $thetable . " WHERE 1 ";

   	if ($leadid ne "") {
      		$mystring = $mystring . " AND id='" . $leadid . "'";
   	}
         
  	if ($phone ne "") {
      		$mystring = $mystring . " AND phone LIKE '%" . $phone . "%'";
   	}

   	if ($name ne "") {
      		$mystring = $mystring . " AND name LIKE '%$name%'";
   	}
   	
  } else {
        $mystring = "SELECT * FROM " . $thetable . " WHERE phone = '9898989898' ";  
  }
  
  $mystring = $mystring . " LIMIT 10";
          
  $sth = $dbh->prepare("$mystring");
  $sth->execute or die("Unable to excute query");
  print "<center>";

print <<EOT;

</center>
EOT
   print "<center>";
   
   $hit_count=0;
   print "<table BORDER=1 CELLSPACING=2 CELLPADDING=2>";
   print "<tr><td>LeadID</td><td>Phone</td><td>Name</td></tr>";
   
   while(@record = $sth->fetchrow_array) {
      $hit_count = $hit_count + 1;
   
      print "<tr>";
      print "<td>$record[0]</td>";
      print "<td>$record[1]</td>";
      print "<td>$record[3]</td>";
  
      print "<td bgcolor=#FFFFFF>";
      print "<a href=\"$mysecureurl$mycgi?fs=edit&table=$thetable&leadid=$record[0]\">";
      print "Edit</a></td>";
      
      print "</tr>"; 
   }
   
   print "</table>";

   $sth->finish;

   print "</table>";

#TESTAREA#####################
print "<table><tr>";

}
#####################################
sub env {
     print "<table>\n";
         foreach $key (sort keys %ENV)
         {
         print qq|<tr><th align="right">$key:</th><td>$ENV{$key}</td></tr>\n|;
         }
     print "</table>\n";
}
#############################################################################
sub passback {
   print "<table>\n";
   foreach $key (sort keys %FORM)
   {
      print qq|<tr><th align="right">$key:</th><td>$FORM{$key}</td></tr>\n|;
   }
   print "</table>\n";
}

##########################
sub myerror {
print "<td ALIGN=TOP>";
print "<h3>Not implemented yet...\n</h3>";
print "</td></tr></table>";
}
##########################
sub fetchit {

if ($ENV{'REQUEST_METHOD'} eq 'GET')
{
    @pairs = split(/&/, $ENV{'QUERY_STRING'});
}
elsif ($ENV{'REQUEST_METHOD'} eq 'POST')
    {
        read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});
        @pairs = split(/&/, $buffer);
    } else {
        #&output("Error 112","Bad or Unknown Request Method");
        print "Bad or Unknown Request Method";
    }

    foreach $pair (@pairs)
        {
        local($name, $value) = split(/=/, $pair);
        $name =~ tr/+/ /;
        $name =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        $value =~ tr/+/ /;
        $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        $value =~ s/<!--(.|\n)*-->//g;

        $INPUT{$name} = $value;
        if (($name =~ /^r_/) && (!$value))
             {
             push(@missing_fields,$name)
             }
        if (($name =~ /email/) && ($value) && (($value =~ /(@.*@)|(\.\.)|(@\.)|(\.@)|(^\.)/) ||
                ($value !~ /^.+\@(\[?)[a-zA-Z0-9\-\.]+\.([a-zA-Z]{2,3}|[0-9]{1,3})(\]?)$/)))
             {
             push(@bad_emails,$name)
                        }
             $FORM{$name} = $value;

             ### BEGIN DEBUG SECTION
#             print "Content-type: text/plain\n\n";
#             print "In debug mode\n";
#             print "$value = $name\n";
#             exit;
             ### END DEBUG SECTION
             }
        }

##########################
sub mail_user {
open (MAIL, "|$mailprog -t")
   || print "Can't start mail program";

print MAIL "To: $email\n";
print MAIL "From: newuser\@dynx.net (DynX Services)\n";
print MAIL "Subject: New User Information\n\n";
print MAIL "-" x 75 . "\n\n";
print MAIL "Your request for a DynX Account was Successful. \n";
print MAIL "As soon as your request is processed, you will be \n";
print MAIL "notified by email that you can login. \n\n";
print MAIL "Username: $username \n";
print MAIL "Password: $password \n\n";
print MAIL "You should file this away somewhere safe! \n";
print MAIL "\n\n";
close (MAIL);
}
##########################
sub mail_dynx {
open (MAIL, "|$mailprog -t")
   || print "Can't start mail program";

print MAIL "To: pchammer\@dynx.net\n";
print MAIL "From: newuser\@dynx.net (DynX Services)\n";
print MAIL "Subject: New User Information\n\n";
print MAIL "-" x 75 . "\n\n";
print MAIL "The following user has signed up for DynX Services. \n";
print MAIL "Username: $username \n";
print MAIL "   Email: $email \n\n";
print MAIL "\n\n";
close (MAIL);
}
##########################
sub getdate {
   my $sth = $dbh->prepare("SELECT LEFT(NOW(),8)");
   $sth->execute;
   ( $now ) = $sth->fetchrow();
   $now =~ s/://g;
   $now =~ s/-//g;
   $now =~ s/\s//g;
   return $now;
}
##########################
sub getdatetime {
   my $sth = $dbh->prepare("SELECT NOW()");
   $sth->execute;
   ( $now ) = $sth->fetchrow();
   $now =~ s/://g;
   $now =~ s/-//g;
   $now =~ s/\s//g;
   return $now;
}
#########################
sub error {
  local($errornum, $error) = @_;
  print "<p><b>The Following Errors Were Encountered!</b>";
  print "<br><b>ERROR CODE</b>: $errornum";
  print "<br><b>ERROR TYPE</b>: $error <BR><hr>";
  print "<p>Press your browser's BACK button to Go back to the form and fix them. Thank you.";
  print "<p>$footer";
  exit;
}

#*************************
