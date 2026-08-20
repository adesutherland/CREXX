#!/usr/bin/env python3
"""Round RCC-5C exact decimal constants independently at caller precisions."""

from decimal import Decimal, ROUND_HALF_UP, localcontext


VALUES = {
    "pi": Decimal(
        "3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808648"
    ),
    "euler": Decimal(
        "2.71828182845904523536028747135266249775724709369995957496696762772407663035354759457138217852516642742746639193"
    ),
}


for digits in (9, 10, 18, 19, 32, 33, 64):
    with localcontext() as context:
        context.prec = digits
        context.rounding = ROUND_HALF_UP
        rounded = " ".join(f"{name}={+value}" for name, value in VALUES.items())
    print(f"digits={digits} {rounded}")
