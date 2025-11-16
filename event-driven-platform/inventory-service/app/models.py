from typing import Optional
from pydantic import BaseModel, Field

class InventoryItem(BaseModel):
    product_id: int
    quantity: int = Field(default=0, ge=0)
    reserved: int = Field(default=0, ge=0)
    warehouse_location: Optional[str] = None
    last_updated: Optional[float] = None
