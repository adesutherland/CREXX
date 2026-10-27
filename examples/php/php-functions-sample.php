<?php
function addTax($amount, $rate) {
    return $amount + $rate;
}

function label($name) {
    return "Customer: " . $name;
}

$total = addTax(100, 19);
echo "label(Harry) expected=Customer: Harry result=" . label("Harry");
echo "addTax(100, 19) expected=119 result=" . $total;
?>
