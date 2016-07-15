<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Price Changes List');

$act = getGP('act');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Price Changes List');
$pg->head('Price Changes List');

$prc = new prices($pg);

if ($act == "print")
{
    // output a printable list of price changes
    if (!strlen($_GET['todate'])) { $_GET['todate'] = date('m/d/Y'); }

    $nu = getG('newused');
    $platformID = getG('platformID');
    $from = strtotime(getG('fromdate'));
    $to = (strtotime(getG('todate')) + (60 * 60 * 24) - 1);
    $limit = getG('limit',NO);

    $nuword = ($nu==ITEM_NEW ? 'new' : ($nu==ITEM_USED ? 'used' : 'new/used'));

    $prc->setPriceChanges($nu,$platformID,$from,$to);
    $changes = $prc->getPriceChanges();

    $changes = array_reverse($changes);
    $byplatform = array(); // format: $byplatform[platform_name] = array(ITEM_NEW=>items,ITEM_USED=>items)
    $seen_itemIDs = array(
        ITEM_NEW  => array(),
        ITEM_USED => array()
    );
    while (list($a,$arr) = each($changes))
    {
        if (!isset($byplatform[$arr['pla_name']]))
        {
            $byplatform[$arr['pla_name']] = array(
                ITEM_NEW  => array(),
                ITEM_USED => array()
            );
        }
        if ($arr['pch_newused'] == ITEM_NEW && $arr['pch_to'] > 0)
        {
            if (!$limit || !in_array($arr['pch_itemID'],$seen_itemIDs[ITEM_NEW]))
            {
                $byplatform[$arr['pla_name']][ITEM_NEW][] = $arr;
                $seen_itemIDs[ITEM_NEW][] = $arr['pch_itemID'];
            }
        }
        if ($arr['pch_newused'] == ITEM_USED && $arr['pch_to'] > 0)
        {
            if (!$limit || !in_array($arr['pch_itemID'],$seen_itemIDs[ITEM_USED]))
            {
                $byplatform[$arr['pla_name']][ITEM_USED][] = $arr;
                $seen_itemIDs[ITEM_USED][] = $arr['pch_itemID'];
            }
        }
    }
    reset($changes);

    $cols = 3;
    $rows = count($byplatform);
    $keys = array_keys($byplatform);
    $width = floor(100/$cols);

    $table_width = 650;
    $col_width = ceil($table_width/$cols)-ceil(10/$cols);

    $max_title_len = 30;
    $price_len = 6;

    ?>
    <style type="text/css">
        .cr { font-family:Courier New;font-size:8pt; }
    </style>

    All <?=$nuword;?> prices changed: <?=date('m/d/Y',$from);?> - <?=date('m/d/Y',$to);?>
    <p />
    <?php
    if (!$limit)
    {
        ?>
        <span class="note">
            <b>Note:</b> Price changes are listed in the order they occurred (the last shown change is the latest change)
        </span>
        <p />
        <?php
    }

    if (count($changes))
    {
        ?>
        <table border="0" bordercolor="#000000" cellspacing="0" cellpadding="2" width="<?=$table_width;?>">
        <?php
        for ($i=0; $i<$rows; $i++)
        {
            $key = @$keys[$i];
            $items = @$byplatform[$key];
            $show = array(
                $items[ITEM_NEW],
                $items[ITEM_USED]
            );

            ?>
            <tr><td colspan="<?=$cols+1;?>" align="center">
                <hr width="100%" size="-1" color="#000000" noshade="noshade" /><br />
                <b><?=$key;?></b>
            </td></tr>
            <?php
            while (list($a,$nu) = each($show))
            {
                if (count($nu))
                {
                    $nu = array_reverse($nu);
                    if ($a && count($show[0])) { ?><tr><td colspan="<?=$cols+1;?>" align="center"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr><?php }
                    ?>
                    <tr height="30">
                        <td background="/images/vert_<?=(!$a?'new':'used');?>.gif" width="10"><img src="/images/blank.gif" width="10" height="1" /></td>
                        <?php
                        if (is_array($nu))
                        {
                            $percol = ceil(count($nu)/$cols);

                            for ($j=0; $j<$cols; $j++)
                            {
                                $start = ($j*$percol);
                                $end = (($j+1)*$percol)-1;

                                ?>
                                <td valign="top" width="<?=$width;?>%">
                                    <table border="0" cellspacing="2" cellpadding="0" width="100%">
                                        <tr>
                                                                                        <td valign="top" style="font-size:9">
                                                <?php
                                                $shown = 0;

                                                for ($k=$start; $k<=$end; $k++)
                                                {
                                                    $arr = @$nu[$k];

                                                    if (is_array($arr))
                                                    {
                                                        $shown++;
                                                        $fromprice = $arr['pch_from'];
                                                        $toprice = $arr['pch_to'];

                                                        $title = $arr['itm_title'];
                                                        if (strlen($title) > $max_title_len) { $title = substr($title,0,($max_title_len-3)).'...'; }

                                                                                                                ?><span class="cr"><?=(strlen($toprice)<$price_len?str_repeat('&nbsp;',($price_len-strlen($toprice))):'').$toprice;?></span> &nbsp; <?=$title;?><?php
                                                        ?><br /><?php
                                                        echo "\n";
                                                    }
                                                }

                                                if (!$shown) { echo "&nbsp;"; }
                                                ?>
                                            </td>
                                        </tr>
                                    </table>
                                </td>
                                <?php
                            }
                        }
                        else { ?><td colspan="<?=$cols;?>">&nbsp;</td><?php }
                    ?></tr><?php
                }
            }
        }
        ?>
        </table>
        <?php

        //$pg->addOnload('window.print()');
    }
    else
    {
        ?>
        No price changes found in selected date range
        <p />
        <input type="button" value="&lt; Select Different Date" onclick="document.location='price_changes.php?newused=<?=$_GET['newused'];?>'" class="btn" />
        <?php
    }
}

$pg->foot();
?>
