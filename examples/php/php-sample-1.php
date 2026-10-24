<?php

$customer = "Peter";
$unitPrice = 20;
$quantity = 3;
$shipping = 5;
$taxRate = 0.19;
$discount = 0;

$subtotal = $unitPrice * $quantity;

if ($subtotal >= 50) {
    $discount = $subtotal * 0.10;
} else {
    $discount = 0;
}

$taxBase = $subtotal - $discount + $shipping;
$tax = $taxBase * $taxRate;
$total = $taxBase + $tax;

echo "Customer: " . $customer;
echo "Quantity: " . $quantity;
echo "Subtotal: " . $subtotal;
echo "Discount: " . $discount;
echo "Shipping: " . $shipping;
echo "Tax: " . $tax;
echo "Total: " . $total;

$position = 1;

while ($position <= $quantity) {
    echo "Processing item " . $position;
    $position = $position + 1;
}

?>
