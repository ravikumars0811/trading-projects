from pydantic import BaseModel, Field
from typing import Optional

class InventoryItemCreate(BaseModel):
    product_id: int = Field(..., gt=0)
    quantity: int = Field(default=0, ge=0)
    reserved: int = Field(default=0, ge=0)
    warehouse_location: Optional[str] = None

class InventoryItemUpdate(BaseModel):
    quantity: Optional[int] = Field(None, ge=0)
    reserved: Optional[int] = Field(None, ge=0)
    warehouse_location: Optional[str] = None

class InventoryItemResponse(BaseModel):
    id: str
    product_id: int
    quantity: int
    reserved: int
    warehouse_location: Optional[str] = None
    last_updated: Optional[float] = None

    class Config:
        from_attributes = True
