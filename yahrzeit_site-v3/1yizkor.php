<?php
/*
 * NAME
 *      1yizkor.php
 *
 * DESCRIPTION
 *      Yizkor schedule summary and manual wall-wide lighting screen for the
 *      CBS Yahrzeit Wall.
 *
 *      This page lists the next enabled Yizkor observances and provides four
 *      immediate controller operations:
 *
 *          - turn all LEDs off
 *          - turn all LEDs on
 *          - restore policy-driven Yahrzeit lighting
 *          - turn on the full-wall Yizkor display
 *
 *      Operations call bin/yahrzeit so this screen uses the same command and
 *      transport path as scheduled automation and command-line operation.
 *
 * BLUF
 *      This page shows upcoming Yizkor dates and provides exceptional live
 *      wall control. Calendar rules remain in date_support.inc.php; this page
 *      does not edit policy or display panel geometry.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/date_support.inc.php";

const YIZKOR_TITLE = "Yizkor and Manual Lighting";
const YIZKOR_DESCRIPTION = "Review upcoming Yizkor dates or change the entire Yahrzeit Wall display immediately.";
const YIZKOR_TAB = 2;
const YIZKOR_HELPFILE = "help/1yizkor.php";

/** Run one permitted wall-wide operation through the shared command wrapper. */
function yizkor_run_operation($operation)
{
    $allowed = [
        "scheduled" => "",
        "all-off"   => "--all-off",
        "all-on"    => "--all-on",
        "yizkor"    => "--yizkor",
    ];

    if (!isset($allowed[$operation])) {
        return [false, "Unsupported lighting operation: $operation"];
    }

    $script = __DIR__ . "/bin/yahrzeit";
    if (!is_executable($script)) {
        return [false, "Yahrzeit command script is not executable: $script"];
    }

    $cmd = escapeshellarg($script);
    if ($allowed[$operation] != "") {
        $cmd .= " " . escapeshellarg($allowed[$operation]);
    }
    $cmd .= " 2>&1";

    exec($cmd, $output, $rc);

    return [$rc == 0, implode("\n", $output)];
}

function yizkor_render_main_page()
{
    $minhag = read_minhag_ini();
    $observances = next_yizkor_observances($minhag);

    emitHeader(YIZKOR_TITLE, YIZKOR_TAB);
    emitTopOfScreen(YIZKOR_TITLE, YIZKOR_DESCRIPTION, YIZKOR_HELPFILE);
?>

    <form name="walllighting" action="<?php echo h($_SERVER['PHP_SELF']); ?>" method="POST">
    <table cellspacing="0" cellpadding="4" width="100%" border="0" class="botBorder">
        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">Next Yizkor events</span>
            </td>
        </tr>

        <tr>
            <td colspan="3" class="text yizkorScheduleCell">
<?php
                emit_yizkor_observance_table($observances);
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">Wall-Wide Lighting Operations</span><br>
                <span class="textSmall">These controls send commands to the wall immediately.</span>
            </td>
        </tr>

        <tr>
            <td colspan="3" class="text lightingControlCell">
                <p>
                    Use these controls only for testing, maintenance, or an
                    intentional operator action during a service.
                </p>

                <div class="lightingButtonGrid">
                    <button type="submit" name="lighting_operation" value="all-off" class="button lightingButton">
                        Turn all lights off
                    </button>

                    <button type="submit" name="lighting_operation" value="all-on" class="button lightingButton">
                        Turn all lights on
                    </button>

                    <button type="submit" name="lighting_operation" value="scheduled" class="button lightingButton">
                        Yahrzeit lights
                    </button>

                    <button type="submit" name="lighting_operation" value="yizkor" class="button lightingButton">
                        Yizkor lights
                    </button>
                </div>

                <p>
                    Normal Yahrzeit and Yizkor lighting is automatic; routine
                    operation does not require this page.
                </p>
            </td>
        </tr>

<?php
        emitCopyright();
?>
    </table>
    </form>

<?php
    emitFooter();
}

function yizkor_render_result_page($operation, $ok, $message)
{
    $title = $ok ? "Lighting operation completed" : "Lighting operation failed";

    emitHeader(YIZKOR_TITLE, YIZKOR_TAB);
    emitTopOfScreen(
        $title,
        "Result from manual lighting operation: " . h($operation),
        YIZKOR_HELPFILE
    );
?>

    <table cellspacing="0" cellpadding="4" width="100%" border="0" class="botBorder">
        <tr>
            <td class="text">
                <pre><?php echo h($title . "\n\n" . $message); ?></pre>
            </td>
        </tr>
        <tr>
            <td class="text">
                <a href="1yizkor.php">Return to the Yizkor lighting page</a>
            </td>
        </tr>
<?php
        emitCopyright();
?>
    </table>

<?php
    emitFooter();
}

function yizkor_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method == 'POST') {
        $operation = $_POST['lighting_operation'] ?? "";
        [$ok, $message] = yizkor_run_operation($operation);
        yizkor_render_result_page($operation, $ok, $message);
        return;
    }

    if ($method == 'GET') {
        yizkor_render_main_page();
        return;
    }

    die("This script only works with GET and POST requests.");
}

yizkor_main();
