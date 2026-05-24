<?php 
    require_once "misc.inc.php";
    require_once "panels.inc.php";
    require_once "names.inc.php";
    $minhag = read_minhag_ini();

    $title = "Yahrzeit Names";
    $description = "List observed Yahrzeits.  Click on a name to view that individual record.";
    $tab = 3;         // Names

    function h($s)
    {
        return htmlspecialchars((string)$s, ENT_QUOTES, "UTF-8");
    }

    function yahrzeit_person_matches_query($person, $query)
    {
        if ($query == "") {
            return true;
        }

        $haystack = strtolower(
            $person['firstName'] . " " .
            $person['lastName'] . " " .
            $person['lastNameFirst'] . " " .
            $person['engYzMonth'] . "/" .
            $person['engYzDD'] . "/" .
            $person['engYzYYYY'] . " " .
            $person['hebYzDD'] . " " .
            $person['hebYzMonth'] . " " .
            $person['hebYzYYYY'] . " " .
            $person['options'] . " " .
            $person['panelId'] . "-" .
            $person['row'] . "-" .
            $person['column']
        );

        $words = preg_split('/\s+/', strtolower(trim($query)));
        if ($words === false) {
            return true;
        }

        foreach ($words as $word) {
            if ($word == "") {
                continue;
            }

            if (strpos($haystack, $word) === false) {
                return false;
            }
        }

        return true;
    }

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        emitHeader( $title, $tab );

        $query = isset($_GET['q']) ? trim($_GET['q']) : "";
?>

<body class="bgNone">

<?php   
    emitTopOfScreen( $title, $description );
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
        <tr><td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class=boldText> Yahrzeit Names </span>
            </td>
        </tr>

        <tr>
            <td colspan=3 align="center">
                <form name="searchnames" action="<?php echo h($_SERVER['PHP_SELF']) ?>" method="GET">
                    <span class="text">Search names, dates, options, or location:</span>
                    <input type="text" name="q" size="40" value="<?php echo h($query) ?>">
                    <input type="submit" value="Search" class="button">
                    <input type="button" value="Clear" class="button"
                           onclick='window.location="4viewnames.php";return false;'>
                </form>
            </td>
        </tr>

        <tr>
            <td colspan=3 align="center">
<?php
                $n = yahrzeit_readDB();
                $displayed = 0;
?>
                <table border=2>
                    <tr class="text">
                        <th>Name</th>
                        <th>English Date</th>
                        <th>Hebrew Date</th>
                        <th>Options</th>
                        <th>Location</th>
                    </tr>

<?php
                    for ( $i = 0; $i < $n; $i++ ) {
                        $remembered = yahrzeit_getObj( $i );

                        if (!yahrzeit_person_matches_query($remembered, $query)) {
                            continue;
                        }
                        $name = trim($remembered['firstName'] . " " . $remembered['lastName']);

                        $englishDate = trim(
                            $remembered['engYzMonth'] . "/" .
                            $remembered['engYzDD'] . "/" .
                            $remembered['engYzYYYY'],
                            "/"
                        );

                        $hebrewDate = trim(
                            $remembered['hebYzDD'] . " " .
                            $remembered['hebYzMonth'] . " " .
                            $remembered['hebYzYYYY']
                        );

                        $options = isset($remembered['options']) ? trim($remembered['options']) : "";

                        $location = trim(
                            $remembered['panelId'] . "-" .
                            $remembered['row'] . "-" .
                            $remembered['column'],
                            "-"
                        );

                        $displayed++;
?>

                        <tr class="text">
                            <td>  <!-- Name -->
                                <a href="5singlename.php?row=<<?php echo h($name) ?></a>">
                                     <?php echo h($name) ?></a></a>
                            </td> 
                            <td> <!-- English Yahrzeit Date -->
                                <?php echo h($englishDate) ?>
                            </td>
                            <td> <!-- Hebrew Yahrzeit Date -->
                                <?php echo h($hebrewDate) ?>
                            </td>
                            <td> <!-- Options -->
                                <?php echo h($options) ?> 
                            </td>
                            <td> <!-- Location -->
                                <?php echo h($location) ?> 
                            </td>
                        </tr>
<?php
                    }

                    if ($displayed == 0) {
?>
                        <tr class="text">
                            <td colspan=5 align="center">
                                <i>No matching Yahrzeit records found.</i>
                            </td>
                        </tr>
<?php
                    }
?>
                </table>
            </td>
        </tr>

        <tr>
            <td colspan=3 align="center" class="textSmall">
                Showing <?php echo h($displayed) ?> of <?php echo h($n) ?> records.
            </td>
        </tr>

        <tr>
            <td colspan="3" height="10"></td>
        </tr>

<?php
        emitCopywrite(); 
?>

    </table>
<br>&nbsp;<br>
</body>

<?php 
        emitFooter();
    }
    else {
        die ("This script only works with GET requests.");
    }

?>
