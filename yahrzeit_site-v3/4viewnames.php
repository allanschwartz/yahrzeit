<?php
/*
 * NAME
 *      4viewnames.php
 *
 * DESCRIPTION
 *      Read-only memorial-name browser for the CBS Yahrzeit Wall.
 *
 *      This page displays the memorial records stored in the live Yahrzeit
 *      CSV database. It provides a simple search box for finding names and
 *      shows the dates, options, and physical LED location associated with
 *      each matching record.
 *
 *      The live memorial database is:
 *
 *          data/yahrzeits-rev4.csv
 *
 *      Memorial records are read through include/names.inc.php so that this
 *      page does not depend directly on the CSV column order.
 *
 * BLUF
 *      This page is for finding and reviewing memorial records.
 *
 *      It should not edit names, calculate lighting schedules, or generate
 *      controller commands.
 *
 * NOTES
 *      This page is intentionally read-only. Database editing, CSV upload,
 *      reports, and audit checks belong on the Reports screen.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized as a read-only searchable browser in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/date_support.inc.php";
require_once "include/names.inc.php";

const VIEWNAMES_TITLE = "Yahrzeit Names";
const VIEWNAMES_DESCRIPTION = "List observed Yahrzeits. Click on a name to view that individual record.";
const VIEWNAMES_TAB = 3;
const VIEWNAMES_HELPFILE = "help/4viewnames.php";
const VIEWNAMES_PAGE = "4viewnames.php";


// -----------------------------------------------------------------------------
// Search and formatting helpers
// -----------------------------------------------------------------------------

function viewnames_english_date($person)
{
    return trim(
        ($person['engYzMonth'] ?? "") . "/" .
        ($person['engYzDD']    ?? "") . "/" .
        ($person['engYzYYYY']  ?? ""),
        "/"
    );
}

function viewnames_hebrew_date($person)
{
    return trim(
        ($person['hebYzDD']    ?? "") . " " .
        ($person['hebYzMonth'] ?? "") . " " .
        ($person['hebYzYYYY']  ?? "")
    );
}

function viewnames_search_text($person)
{
    return strtolower(implode(" ", [
        yahrzeit_person_name($person),
        $person['lastNameFirst'] ?? "",
        viewnames_english_date($person),
        viewnames_hebrew_date($person),
        yahrzeit_person_options_text($person),
        $person['options'] ?? "",
        yahrzeit_person_location($person),
    ]));
}

function viewnames_person_matches_query($person, $query)
{
    $query = strtolower(trim($query));

    if ($query == "") {
        return true;
    }

    $haystack = viewnames_search_text($person);
    $words = preg_split('/\s+/', $query) ?: [];

    foreach ($words as $word) {
        if ($word != "" && !str_contains($haystack, $word)) {
            return false;
        }
    }

    return true;
}


// -----------------------------------------------------------------------------
// Rendering helpers
// -----------------------------------------------------------------------------

function viewnames_render_search_form($query)
{
?>
                <form name="searchnames" action="<?php echo h(VIEWNAMES_PAGE); ?>" method="GET">
                    <span class="text">Search names, dates, options, or location:</span>
                    <input type="text" name="q" size="40" value="<?php echo h($query); ?>">
                    <input type="submit" value="Search" class="button">
                    <input type="button" value="Clear" class="button"
                           onclick='window.location="<?php echo h(VIEWNAMES_PAGE); ?>";return false;'>
                </form>
<?php
}

function viewnames_render_names_table($query)
{
    $n = yahrzeit_readDB();
    $displayed = 0;
?>
                <table border="2">
                    <tr class="text">
                        <th>Name</th>
                        <th>English Date</th>
                        <th>Hebrew Date</th>
                        <th>Options</th>
                        <th>Location</th>
                    </tr>
<?php
    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);

        if (!viewnames_person_matches_query($person, $query)) {
            continue;
        }

        $displayed++;
?>
                    <tr class="text">
                        <td>
                            <a href="5singlename.php?row=<?php echo h($i); ?>">
                                <?php echo h(yahrzeit_person_name($person)); ?>
                            </a>
                        </td>
                        <td><?php echo h(viewnames_english_date($person)); ?></td>
                        <td><?php echo h(viewnames_hebrew_date($person)); ?></td>
                        <td><?php echo h(yahrzeit_person_options_text($person)); ?></td>
                        <td><?php echo h(yahrzeit_person_location($person)); ?></td>
                    </tr>
<?php
    }

    if ($displayed == 0) {
?>
                    <tr class="text">
                        <td colspan="5" align="center">
                            <i>No matching Yahrzeit records found.</i>
                        </td>
                    </tr>
<?php
    }
?>
                </table>
                <br>
                <span class="textSmall">
                    Showing <?php echo h($displayed); ?> of <?php echo h($n); ?> records.
                </span>
<?php
}

function viewnames_render_main_page()
{
    $query = trim($_GET['q'] ?? "");

    emitHeader(VIEWNAMES_TITLE, VIEWNAMES_TAB);
    emitTopOfScreen(VIEWNAMES_TITLE, VIEWNAMES_DESCRIPTION, VIEWNAMES_HELPFILE);
?>
    <table cellspacing="0" cellpadding="4" width="90%" border="0" class="botBorder">
        <tr>
            <td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">Yahrzeit Names</span>
            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
<?php
                viewnames_render_search_form($query);
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
<?php
                viewnames_render_names_table($query);
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" height="10"></td>
        </tr>

<?php
        emitCopyright();
?>
    </table>
    <br>&nbsp;<br>
<?php
    emitFooter();
}


// -----------------------------------------------------------------------------
// Program entry point
// -----------------------------------------------------------------------------

function viewnames_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method == 'GET') {
        viewnames_render_main_page();
        return;
    }

    die("This script only works with GET requests.");
}

viewnames_main();
